/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#include "systemtray.h"
#include "mainwindow.h"
#include "menudecorator.h"
#include <core/log.h>
#include <settings/appsettings.h>
#include <common/guiutils.h>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>


namespace m4e
{
namespace gui
{

static const QString M4E_SYSTRAY_ICON_NORMAL = ":/icon-tray.png";
static const QString M4E_SYSTRAY_ICON_NOTIFY = ":/icon-tray-notify.png";

enum MenuIDs
{
    MenuOpen                = 100,
    MenuQuit                = 101,
    MenuEnableNotification  = 102,
    MenuEnableAlarm         = 103,
    MenuEnableAutoStart     = 104
};


SystemTray::SystemTray( webapp::WebApp* p_webApp, MainWindow* p_parent ) :
 QObject( p_parent ),
 _p_webApp( p_webApp ),
 _p_mainWindow( p_parent )
{
    setupSystemTray();

    connect( _p_webApp, SIGNAL( onUserSignedIn( bool, QString ) ), this, SLOT( onUserSignedIn( bool, QString ) ) );
    connect( _p_webApp, SIGNAL( onUserSignedOff( bool ) ), this, SLOT( onUserSignedOff( bool ) ) );
    connect( _p_webApp, SIGNAL( onServerConnectionClosed() ), this, SLOT( onServerConnectionClosed() ) );

    connect( _p_webApp->getNotifications(), SIGNAL( onEventMessage( QString, QString, QString, m4e::notify::NotifyEventPtr ) ), this,
                                            SLOT( onEventMessage( QString, QString, QString, m4e::notify::NotifyEventPtr ) ) );

    connect( _p_webApp->getNotifications(), SIGNAL( onEventLocationVote( QString, QString, QString, QString, bool ) ), this,
                                            SLOT( onEventLocationVote( QString, QString, QString, QString, bool ) ) );

    connect( _p_webApp->getMailBox(), SIGNAL( onResponseCountUnreadMails( bool, int ) ), this,
                                      SLOT( onResponseCountUnreadMails( bool, int ) ) );

    connect( _p_webApp->getEvents(), SIGNAL( onLocationVotingStart( m4e::event::ModelEventPtr ) ), this,
                                     SLOT( onLocationVotingStart( m4e::event::ModelEventPtr ) ) );

    connect( _p_webApp->getEvents(), SIGNAL( onLocationVotingEnd( m4e::event::ModelEventPtr ) ), this,
                                     SLOT( onLocationVotingEnd( m4e::event::ModelEventPtr ) ) );

    connect( _p_webApp->getChatSystem(), SIGNAL( onReceivedChatMessageEvent( m4e::chat::ChatMessagePtr ) ), this,
                                     SLOT( onReceivedChatMessageEvent( m4e::chat::ChatMessagePtr ) ) );

    connect( _p_webApp->getChatSystem(), SIGNAL( onReceivedChatMessageUser( m4e::chat::ChatMessagePtr ) ), this,
                                     SLOT( onReceivedChatMessageUser( m4e::chat::ChatMessagePtr ) ) );
}

SystemTray::~SystemTray()
{
}

void SystemTray::onActivated( QSystemTrayIcon::ActivationReason reason )
{
    switch ( reason )
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            showIconNotify( false );
            common::GuiUtils::bringWidgetToFront( _p_mainWindow );
        break;

        default:
            break;
    }
}

void SystemTray::onMenuTriggert( QAction* p_action )
{
    int id = p_action->data().toInt();
    switch( id )
    {
        case MenuOpen:
            common::GuiUtils::bringWidgetToFront( _p_mainWindow );
        break;

        case MenuQuit:
            _p_mainWindow->terminate();
        break;

        case MenuEnableNotification:
            _enableNotification = p_action->isChecked();
            settings::AppSettings::get()->writeSettingsValue( M4E_SETTINGS_CAT_NOTIFY, M4E_SETTINGS_KEY_NOTIFY_EVENT, _enableNotification ? "yes" : "no" );
        break;

        case MenuEnableAlarm:
            _enableAlarm = p_action->isChecked();
            settings::AppSettings::get()->writeSettingsValue( M4E_SETTINGS_CAT_NOTIFY, M4E_SETTINGS_KEY_NOTIFY_ALARM, _enableAlarm ? "yes" : "no" );
        break;

        case MenuEnableAutoStart:
            settings::AppSettings::get()->writeSettingsValue( M4E_SETTINGS_CAT_APP, M4E_SETTINGS_KEY_APP_AUTOSTART, p_action->isChecked() ? "yes" : "no" );
        break;

        default:
            log_warning << TAG << "unsupported tray menu option: " << id << std::endl;
    }
    showIconNotify( false );
}

void SystemTray::onMenuAboutToShow()
{
    showIconNotify( false );
}

void SystemTray::onMessageClicked()
{
    common::GuiUtils::bringWidgetToFront( _p_mainWindow );
}

void SystemTray::setupSystemTray()
{
    QString enablealarm = settings::AppSettings::get()->readSettingsValue( M4E_SETTINGS_CAT_NOTIFY, M4E_SETTINGS_KEY_NOTIFY_ALARM, "yes" );
    QString enableevent = settings::AppSettings::get()->readSettingsValue( M4E_SETTINGS_CAT_NOTIFY, M4E_SETTINGS_KEY_NOTIFY_EVENT, "yes" );
    QString autostart   = settings::AppSettings::get()->readSettingsValue( M4E_SETTINGS_CAT_APP, M4E_SETTINGS_KEY_APP_AUTOSTART, "yes" );

    _enableNotification = ( enableevent == "yes" );
    _enableAlarm = ( enablealarm == "yes" );
    bool enableautostart = ( autostart == "yes" );

    _p_systemTray = new QSystemTrayIcon( QIcon( M4E_SYSTRAY_ICON_NORMAL ), this );
    _p_systemTray->setToolTip( QApplication::translate( "SystemTray", M4E_APP_NAME ) );
    connect( _p_systemTray, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ), this, SLOT( onActivated( QSystemTrayIcon::ActivationReason ) ) );
    connect( _p_systemTray, SIGNAL( messageClicked() ), this, SLOT( onMessageClicked() ) );

    QMenu* p_menu = new QMenu();
    connect( p_menu, SIGNAL( triggered( QAction* ) ), this, SLOT( onMenuTriggert( QAction* ) ) );
    connect( p_menu, SIGNAL( aboutToShow() ), this, SLOT( onMenuAboutToShow() ) );

    MenuDecorator menudeco( p_menu );

    menudeco.addMenuSeperator();
    menudeco.addMenuItem( QApplication::translate( "SystemTray", "Open Meet4Eat" ), QVariant( MenuOpen ) );
    menudeco.addMenuSeperator();
    menudeco.addMenuItemCheckbox( QApplication::translate( "SystemTray", "Enable Notification" ), QVariant( MenuEnableNotification ), _enableNotification );
    menudeco.addMenuItemCheckbox( QApplication::translate( "SystemTray", "Enable Alarm" ), QVariant( MenuEnableAlarm ), _enableAlarm );

    //! NOTE currently there is no support for auto-start on MacOS
#ifndef Q_OS_MACOS
    menudeco.addMenuSeperator();
    menudeco.addMenuItemCheckbox( QApplication::translate( "SystemTray", "Start on Logon" ), QVariant( MenuEnableAutoStart ), enableautostart );
#endif

    menudeco.addMenuSeperator();
    menudeco.addMenuItem( QApplication::translate( "SystemTray", "Quit" ), QVariant( MenuQuit ) );

    _p_systemTray->setContextMenu( p_menu );
    _p_systemTray->show();
}

void SystemTray::showMessage( const QString& title, const QString& message, bool warning, int duration )
{
    // are notifications enabled?
    if ( !_enableNotification )
        return;

    _p_systemTray->showMessage( title, message, warning ? QSystemTrayIcon::Warning : QSystemTrayIcon::Information, duration );
    showIconNotify( true );
}

void SystemTray::showIconNotify( bool show, const QString& toolTip )
{
    // ignore the request if the main window is not minimized
    if ( !( _p_mainWindow->isMinimized() || _p_mainWindow->isHidden() ) )
        return;

    _p_systemTray->setIcon( QIcon( show ? M4E_SYSTRAY_ICON_NOTIFY : M4E_SYSTRAY_ICON_NORMAL ) );
    _p_systemTray->setToolTip( toolTip );
}

bool SystemTray::isTrayAvailable()
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void SystemTray::onUserSignedIn( bool success, QString /*userId*/ )
{
    QString title = QApplication::translate( "SystemTray", "Meet4Eat - User Sign In" );
    QString text;
    bool    warning = false;
    if ( !success )
    {
        warning = true;
        text =  QApplication::translate( "SystemTray", "User could not sign in!" );
    }
    else
    {
        text = QApplication::translate( "SystemTray", "User was successfully signed in." );
    }
    showMessage( title, text, warning );
    showIconNotify( false );
}

void SystemTray::onUserSignedOff( bool /*success*/ )
{
}

void SystemTray::onServerConnectionClosed()
{
}

void SystemTray::onEventMessage( QString senderId, QString /*senderName*/, QString eventId, notify::NotifyEventPtr notify )
{
    QString eventname;
    QString userid;
    event::ModelEventPtr event = _p_webApp->getEvents()->getUserEvent( eventId );
    user::ModelUserPtr   user  = _p_webApp->getUser()->getUserData();

    if ( event.valid() )
        eventname = event->getName();

    if ( user.valid() )
        userid = user->getId();

    // suppress echo
    if ( userid == senderId )
        return;

    QString title = QApplication::translate( "SystemTray", "Meet4Eat - Event Notification" );
    QString text = notify->getSubject();

    showMessage( title, text, false );
}

void SystemTray::onResponseCountUnreadMails( bool success, int count )
{
    if ( success && ( count > 0 ) )
    {
        QString title = QApplication::translate( "SystemTray", "Meet4Eat - New Mails" );
        QString text = QApplication::translate( "SystemTray", "You have received new mails." );
        showMessage( title, text, false );
        showIconNotify( true, text );
    }
}

void SystemTray::onEventLocationVote( QString senderId, QString senderName, QString eventId, QString locationId, bool /*vote*/ )
{
    // check if notifications are enabled
    if ( !_enableNotification )
        return;

    // suppress echo
    QString userid = _p_webApp->getUser()->getUserData()->getId();
    if ( senderId == userid )
        return;

    QString locationname;
    event::ModelEventPtr event = _p_webApp->getEvents()->getUserEvent( eventId );
    if ( event.valid() )
    {
        event::ModelLocationPtr loc = event->getLocation( locationId );
        if ( loc.valid() )
            locationname = loc->getName();
    }

    QString title = QApplication::translate( "SystemTray", "Meet4Eat - New Vote from " ) + senderName;
    QString text = QApplication::translate( "SystemTray", "Location: " ) + " " + locationname;
    showMessage( title, text, false );
}

void SystemTray::onLocationVotingStart( event::ModelEventPtr event )
{
    QString title = QApplication::translate( "SystemTray", "Meet4Eat - Voting Time" );
    QString text = QApplication::translate( "SystemTray", "Event" ) + " " + event->getName();
    showMessage( title, text, false );
}

void SystemTray::onLocationVotingEnd( m4e::event::ModelEventPtr event )
{
    QString title = QApplication::translate( "SystemTray", "Meet4Eat - End of Voting Time" );
    QString text = QApplication::translate( "SystemTray", "Event" ) + " " + event->getName();
    showMessage( title, text, false );
}

void SystemTray::onReceivedChatMessageUser( chat::ChatMessagePtr /*msg*/ )
{
    showIconNotify( true, QApplication::translate( "SystemTray", "New message arrived" ) );
}

void SystemTray::onReceivedChatMessageEvent( chat::ChatMessagePtr /*msg*/ )
{
    showIconNotify( true, QApplication::translate( "SystemTray", "New message arrived" ) );
}

} // namespace gui
} // namespace m4e
