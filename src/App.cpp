#include <pigaco/App.hpp>

#include <chrono>
#include <thread>

#include <pigaco/DirectoryScanner.hpp>
#include <pigaco/Players.hpp>

#include <piga/Definitions.hpp>
#include <pigaco/Game.hpp>

#include <QQmlContext>
#include <QApplication>
#include <QtQml>
#include <QDir>

#include <QCommandLineParser>

#define ELPP_NO_DEFAULT_LOG_FILE
#include <easylogging++.h>

#include <Wt/WServer>
#include <pigaco/WebAdmin.hpp>

#include <pigaco/packaging/Package.hpp>
#include <pigaco/packaging/PackageManager.hpp>

#include <boost/filesystem.hpp>

#include <yaml-cpp/yaml.h>

INITIALIZE_EASYLOGGINGPP

namespace pigaco
{
    App::App()
    {

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

        m_guiApplication = new QGuiApplication(argc, argv);

        //Parse command line arguments for eventual operations.
        QCommandLineParser cmdParser;
        cmdParser.setApplicationDescription("This is the main executable of the piga project. This          \n"
                                            "program aims to create an environment for developers to        \n"
                                            "play and publish their games on arcade-console-like            \n"
                                            "machines. These machines can be easily managed by store-owners \n"
                                            "too, if they like to do so.\n"
                                            "\n"
                                            "For more information, please see the GitHub page or the        \n"
                                            "documentation distributed with this program.                   \n"
                                            "\n"
                                            "GitHub: https://github.com/maximaximal/piga\n"
                                            "\n"
                                            "Compiled with piga-library version: " + QString::number(PIGA_MAJOR_VERSION) + "."
                                            + QString::number(PIGA_MINOR_VERSION) + "." + QString::number(PIGA_MINI_VERSION));

        QCommandLineOption helpOption = cmdParser.addHelpOption();

        QCommandLineOption packFolderOption("pack-folder", 
                m_guiApplication->translate("main", "Pack the specified folder."),
                m_guiApplication->translate("main", "package folder"));
        QCommandLineOption installPackageOption("install-package", 
                m_guiApplication->translate("main", "Install a package."),
                m_guiApplication->translate("main", "path to the PPK file or the directory."));
        QCommandLineOption packageNameOption("package-name", 
                m_guiApplication->translate("main", "The package name. (optional)"),
                m_guiApplication->translate("main", "package name"));
        QCommandLineOption packageVersionOption("package-version", 
                m_guiApplication->translate("main", "The package version. (optional)"),
                m_guiApplication->translate("main", "package version"));
        QCommandLineOption packageAuthorOption("package-author", 
                m_guiApplication->translate("main", "The package version. (optional)"),
                m_guiApplication->translate("main", "package author"));
        QCommandLineOption packageOutputOption("package-output", 
                m_guiApplication->translate("main", "The package output file. (required when creating packages)"),
                m_guiApplication->translate("main", "package output file"));
        QCommandLineOption packageReadOption("read-package", 
                m_guiApplication->translate("main", "Reads a package and lists the information found about it."),
                m_guiApplication->translate("main", "Path to the package to read."));

        cmdParser.addOption(packFolderOption);
        cmdParser.addOption(installPackageOption);
        cmdParser.addOption(packageNameOption);
        cmdParser.addOption(packageVersionOption);
        cmdParser.addOption(packageAuthorOption);
        cmdParser.addOption(packageOutputOption);
        cmdParser.addOption(packageReadOption);

        cmdParser.parse(m_guiApplication->arguments());

        if(cmdParser.isSet("pack-folder"))
        {
            LOG(INFO) << "[PACKAGING] Folder packaging mode selected.";

            if(!cmdParser.isSet("package-output"))
            {
                LOG(FATAL) << "[PACKAGING] An output filename has to be defined!";
                return;
            }

            std::shared_ptr<packaging::Package> package = std::make_shared<packaging::Package>();
            package->fromDirectory(cmdParser.value("pack-folder").toStdString());

            package->setConfigVar(packaging::Package::Directory, cmdParser.value("pack-folder").toStdString());
            if(cmdParser.isSet("package-name"))
            {
                package->setConfigVar(packaging::Package::Name, cmdParser.value("package-name").toStdString());
            }
            if(cmdParser.isSet("package-version"))
            {
                package->setConfigVar(packaging::Package::Version, cmdParser.value("package-version").toStdString());
            }
            if(cmdParser.isSet("package-author"))
            {
                package->setConfigVar(packaging::Package::Author, cmdParser.value("package-author").toStdString());
            }

            package->saveToPPK(cmdParser.value("package-output").toStdString());

            return;
        }
        if(cmdParser.isSet("read-package"))
        {
            LOG(INFO) << "[PACKAGING] Package reading mode selected.";

            std::shared_ptr<packaging::Package> package = std::make_shared<packaging::Package>();

            package->fromPPK(cmdParser.value("read-package").toStdString());

            LOG(INFO) << "[PACKAGING] The name of the package in \"" << cmdParser.value("read-package").toStdString() << "\" is \""
                      << package->getConfigVar(packaging::Package::Name) << "\"";

            if(package->flagActive(packaging::Package::HasAuthor))
                LOG(INFO) << "[PACKAGING] The author is \"" << package->getConfigVar(packaging::Package::Author) << "\"";

            if(package->flagActive(packaging::Package::HasVersion))
                LOG(INFO) << "[PACKAGING] The version is \"" << package->getConfigVar(packaging::Package::Version) << "\", which equates to \"" << package->getVersion().asString() << "\".";

            return;
        }
        if(cmdParser.isSet("install-package"))
        {
            LOG(INFO) << "[PACKAGING] Package install mode selected.";
            std::shared_ptr<packaging::PackageManager> pkgManager = std::make_shared<packaging::PackageManager>();
            packaging::Package *package = new packaging::Package();
            package->fromPath(cmdParser.value("install-package").toStdString());
            
            LOG(INFO) << "[PACKAGING] Trying to install package.";
            pkgManager->installPackage(package);
            
            return;
        }
        if(cmdParser.isSet(helpOption))
        {
            cmdParser.showHelp();
            return;
        }

        //Further initialize the app.
        LOG(INFO) << "Initializing PiGaCo App-Class.";
        m_qmlApplicationEngine = new QQmlApplicationEngine();

        std::string configPath = "";

        if(boost::filesystem::exists("./config.yml")) {
            configPath = "./config.yml";
        } else if(boost::filesystem::exists("/usr/local/etc/piga_host_config.yml")) {
            configPath = "/usr/local/etc/piga_host_config.yml";
        } else if(boost::filesystem::exists("/etc/piga_host_config.yml")) {
            configPath = "/etc/piga_host_config.yml";
        } else {
            //Just use the default if nothing exists anywhere...
            configPath = "./config.yml";
        }

        m_playerManager = std::make_shared<piga::PlayerManager>();
        m_host = std::make_shared<piga::Host>(configPath, m_playerManager);

        std::string packagesConfigFile = "Packages.yml";
        std::string packagesPath = "Games";

        //Try to read configuration file.
        try {
            YAML::Node doc = YAML::LoadAllFromFile(configPath).front();

            if(doc["PackagesPath"]) {
                packagesPath = doc["PackagesPath"].as<std::string>();
            } else {
                LOG(INFO) << "No \"PackagesPath\" property set in \"" << configPath << "\". Using \"" << packagesPath << "\".";
            }
            if(doc["PackagesConfigFile"]) {
                packagesConfigFile = doc["PackagesConfigFile"].as<std::string>();
            } else {
                LOG(INFO) << "No \"PackagesConfigFile\" property set in \"" << configPath << "\". Using \"" << packagesConfigFile << "\".";
            }
            if(doc["DataPath"]) {
                m_dataPath = doc["DataPath"].as<std::string>();
            } else {
                LOG(INFO) << "No \"DataPath\" property set in \"" << configPath << "\". Using \"" << m_dataPath << "\".";
            }
        } catch(YAML::BadFile &e) {
            LOG(INFO) << "Config file is not readable!";
        }

        LOG(INFO) << "Starting the piga host.";
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

        LOG(DEBUG) << "Starting the package manager.";
        m_packageManager = std::make_shared<packaging::PackageManager>();
        LOG(DEBUG) << "Reading package data.";
        m_packageManager->readData(packagesConfigFile, packagesPath);

        LOG(DEBUG) << "Starting the directory scanner with data from the package manager.";
        m_directoryScanner = std::make_shared<DirectoryScanner>(m_host);
        m_directoryScanner->setHost(m_host);
        m_directoryScanner->readPackages(m_packageManager);

        qmlRegisterType<Game>("com.pigaco.managing", 1, 0, "Game");
        m_qmlApplicationEngine->rootContext()->setContextProperty("dirScanner", m_directoryScanner.get());
        m_qmlApplicationEngine->rootContext()->setContextProperty("playerManager", m_players.get());
        m_qmlApplicationEngine->addImportPath((m_dataPath + "/forms/").c_str());

        m_qmlApplicationEngine->load(QUrl::fromLocalFile((m_dataPath + "/forms/MainMenu.qml").c_str()));

        QObject *topLevel = m_qmlApplicationEngine->rootObjects().value(0);
        m_qQuickWindow = qobject_cast<QQuickWindow*>(topLevel);

        QSurfaceFormat format;
        format.setSamples(8);
        m_qQuickWindow->setFormat(format);

        m_qQuickWindow->showFullScreen();

        LOG(DEBUG) << "Creating the Wt HTTP server.";

        try {
            m_webServer = new Wt::WServer("Pigaco", m_dataPath + "/Config/wt_config.xml");
            m_webServer->setServerConfiguration(argc, argv, m_dataPath + "/Config/wthttpd.conf");
            m_webServer->setAppRoot(m_dataPath + "/Docroot/");

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

        LOG(DEBUG) << "Starting the App-Loop.";
        m_loopTimer->start(16);

        LOG(DEBUG) << "Starting the QApplication.";
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
    std::shared_ptr<packaging::PackageManager> App::getPackageManager()
    {
        return m_packageManager;
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
                    QCoreApplication::postEvent(m_qQuickWindow, new QKeyEvent(type, Qt::Key::Key_Up, Qt::NoModifier));
                    break;
                case piga::DOWN:
                    QCoreApplication::postEvent(m_qQuickWindow, new QKeyEvent(type, Qt::Key::Key_Down, Qt::NoModifier));
                    break;
                case piga::LEFT:
                    QCoreApplication::postEvent(m_qQuickWindow, new QKeyEvent(type, Qt::Key::Key_Left, Qt::NoModifier));
                    break;
                case piga::RIGHT:
                    QCoreApplication::postEvent(m_qQuickWindow, new QKeyEvent(type, Qt::Key::Key_Right, Qt::NoModifier));
                    break;
                case piga::ACTION:
                    QCoreApplication::postEvent(m_qQuickWindow, new QKeyEvent(type, Qt::Key::Key_Return, Qt::NoModifier));
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
        else if(gameEvent.type() == piga::GameEvent::ChangeOutput)
        {
            m_cacheGameEvent = gameEvent;
            m_host->pushChangeOutputEvent(m_cacheGameEvent.outputChanged);
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

        m_playerManager->resetEdited();
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
