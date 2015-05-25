#include <pigaco/App.hpp>

#include <chrono>
#include <thread>

#include <pigaco/DirectoryScanner.hpp>
#include <pigaco/Players.hpp>

#include <pigaco/Game.hpp>

#include <QQmlContext>
#include <QApplication>
#include <QDir>

#define ELPP_NO_DEFAULT_LOG_FILE
#include <../../include/easylogging++.h>

#include <Wt/WServer>
#include <pigaco/WebAdmin.hpp>

INITIALIZE_EASYLOGGINGPP

namespace pigaco
{
    App::App()
    {
        LOG(INFO) << "Initializing PiGaCo app.";
    }
    App::~App()
    {
        if(m_qmlApplicationEngine != nullptr)
            delete m_qmlApplicationEngine;
        if(m_guiApplication != nullptr)
            delete m_guiApplication;
        if(m_webServer != nullptr)
            delete m_webServer;
    }
    void App::run(int argc, char* argv[])
    {
        LOG(INFO) << "Starting PiGaCo.";

        m_guiApplication = new QGuiApplication(argc, argv);
        m_qmlApplicationEngine = new QQmlApplicationEngine();

        m_playerManager = std::make_shared<piga::PlayerManager>();
        m_host = std::make_shared<piga::Host>("config.yml", m_playerManager);

        m_host->init();

        m_playerManager->init();

        m_playerManager->getPlayer(0)->setName("Player 1");
        m_playerManager->getPlayer(0)->setActive(true);
        m_playerManager->getPlayer(1)->setName("Player 2");
        m_playerManager->getPlayer(1)->setActive(true);

        m_players = std::make_shared<Players>(m_playerManager);

        m_gameInput = std::make_shared<piga::GameInput>();
        
        m_host->setBackcallingGameInput(m_gameInput);

        LOG(INFO) << "Clock-Precision: " << std::chrono::high_resolution_clock::period::den;

        std::chrono::time_point<std::chrono::high_resolution_clock> frameTimePointPast = std::chrono::high_resolution_clock::now();
        std::chrono::time_point<std::chrono::high_resolution_clock> frameTimePoint = std::chrono::high_resolution_clock::now();
        float frametime = 0;
        std::chrono::milliseconds desiredFrametime((long) (1 / 60.f * 1000));

        m_directoryScanner = std::make_shared<DirectoryScanner>(m_host);
        m_directoryScanner->scanDirectory("Games");
        m_directoryScanner->setHost(m_host);

        qmlRegisterType<Game>("com.pigaco.managing", 1, 0, "Game");
        m_qmlApplicationEngine->rootContext()->setContextProperty("dirScanner", m_directoryScanner.get());
        m_qmlApplicationEngine->rootContext()->setContextProperty("playerManager", m_players.get());
        m_qmlApplicationEngine->addImportPath("Data/forms/");

        m_qmlApplicationEngine->load(QUrl::fromLocalFile("Data/forms/MainMenu.qml"));

        QObject *topLevel = m_qmlApplicationEngine->rootObjects().value(0);
        m_qQuickWindow = qobject_cast<QQuickWindow*>(topLevel);

        QSurfaceFormat format;
        format.setSamples(8);
        m_qQuickWindow->setFormat(format);

        m_qQuickWindow->showFullScreen();

        LOG(DEBUG) << "Creating the Wt HTTP server.";

        try {
            m_webServer = new Wt::WServer("Pigaco", "Data/Config/wt_config.xml");
            m_webServer->setServerConfiguration(argc, argv, "Data/Config/wthttpd.conf");

            boost::function<Wt::WApplication* (const Wt::WEnvironment&)> createServer = [&](const Wt::WEnvironment &env){
                LOG(DEBUG) << "Creating the HTTP admin-app class. (Server successfully initialised.)";
                WebAdmin *admin = new WebAdmin(env, this);
                return admin;
            };
            m_webServer->addEntryPoint(Wt::Application, createServer);
            LOG(DEBUG) << "Starting the Wt HTTP server.";
            m_webServer->start();
        } catch(Wt::WServer::Exception &e) {
            LOG(WARNING) << "The Wt HTTP server could not be started/created and has thrown an exception:\n" << e.what();
            delete m_webServer;
            m_webServer = nullptr;
            LOG(DEBUG) << "Cleaned up the Wt Webserver classes after the crash.";
        }


        m_loopTimer = new QTimer(this);

        connect(m_loopTimer, &QTimer::timeout, this, &App::update);
        connect(m_guiApplication, &QGuiApplication::aboutToQuit, this, &App::aboutToQuit);

        LOG(INFO) << "Starting the App-Loop.";
        m_loopTimer->start(16);

        LOG(INFO) << "Starting the QApplication.";
        m_guiApplication->exec();
    }
    void App::onUpdate(float frametime)
    {
        if(!m_isSleeping)
        {

        }
        
        if(m_isSleeping && !m_host->gameIsRunning())
        {
            wakeupWindow();
        }
        else if(!m_isSleeping && m_host->gameIsRunning())
        {
            sleepWindow();
        }
    }
    bool App::end()
    {
        return m_end;
    }
    void App::sleepWindow()
    {
        m_isSleeping = true;
        m_qQuickWindow->setKeyboardGrabEnabled(false);
    }
    void App::wakeupWindow()
    {
        m_isSleeping = false;
        m_qQuickWindow->setKeyboardGrabEnabled(true);
        m_qQuickWindow->raise();
        m_qQuickWindow->requestActivate();
    }
    std::shared_ptr<piga::Host> App::getHost()
    {
        return m_host;
    }
    std::shared_ptr<piga::GameInput> App::getGameInput()
    {
        return m_gameInput;
    }
    std::shared_ptr<piga::PlayerManager> App::getPlayerManager()
    {
        return m_playerManager;
    }
    std::shared_ptr<DirectoryScanner> App::getDirectoryScanner()
    {
        return m_directoryScanner;
    }
    std::shared_ptr<Players> App::getPlayers()
    {
        return m_players;
    }
    void App::setEnd(bool state)
    {
        m_end = state;
    }
    void App::onGameEvent(const piga::GameEvent &gameEvent, float frametime)
    {
        if(gameEvent.type() == piga::GameEvent::GameInput)
        {
            QEvent::Type type;
            if(gameEvent.gameInput.state())
            {
                type = QEvent::KeyPress;
            }
            else
            {
                type = QEvent::KeyRelease;
            }
            switch(gameEvent.gameInput.control())
            {
                case piga::UP:
                    QCoreApplication::postEvent(m_qmlApplicationEngine, new QKeyEvent(type, Qt::Key::Key_Up, Qt::NoModifier));
                    break;
                case piga::DOWN:
                    QCoreApplication::postEvent(m_qmlApplicationEngine, new QKeyEvent(type, Qt::Key::Key_Down, Qt::NoModifier));
                    break;
                case piga::LEFT:
                    QCoreApplication::postEvent(m_qmlApplicationEngine, new QKeyEvent(type, Qt::Key::Key_Left, Qt::NoModifier));
                    break;
                case piga::RIGHT:
                    QCoreApplication::postEvent(m_qmlApplicationEngine, new QKeyEvent(type, Qt::Key::Key_Right, Qt::NoModifier));
                    break;
                case piga::ACTION:
                    QCoreApplication::postEvent(m_qmlApplicationEngine, new QKeyEvent(type, Qt::Key::Key_Return, Qt::NoModifier));
                    break;

                case piga::BUTTON1:
                    break;
                case piga::BUTTON2:
                    break;
                case piga::BUTTON3:
                    break;
                case piga::BUTTON4:
                    break;
                case piga::BUTTON5:
                    break;
                case piga::BUTTON6:
                    break;
                default:
                    //Event unhandled.
                    break;
            }
        }
    }

    void App::aboutToQuit()
    {
        this->setEnd(true);
    }

    void App::update()
    {
        piga::GameEvent gameEvent;
        m_gameInput->update();
        m_host->update(0.016);
        while(m_gameInput->pollEvent(gameEvent))
        {
            if(!m_isSleeping)
            {
                onGameEvent(gameEvent, 0.016);
            }
        }
        onUpdate(0.016);
    }
}

int main(int argv, char* argc[])
{
    START_EASYLOGGINGPP(argv, argc);

    el::Configurations conf;

    conf.setToDefault();
    conf.setGlobally(el::ConfigurationType::ToFile, "false");

    el::Loggers::reconfigureAllLoggers(conf);

    pigaco::App *app = new pigaco::App();

    app->run(argv, argc);

    delete app;

    return 0;
}
