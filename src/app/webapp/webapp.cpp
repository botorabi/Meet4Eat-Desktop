/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#include "webapp.h"
#include <core/log.h>
#include <settings/appsettings.h>
#include <common/dialogmessage.h>
#include <user/userauth.h>
#include <user/user.h>
#include <QApplication>
#include <webapp/m4e-api/m4e-restops.h>
#include <webapp/request/rest-appinfo.h>


namespace m4e
{
namespace webapp
{

WebApp::WebApp( QObject* p_parent ) :
 QObject( p_parent )
{
}

WebApp::~WebApp()
{
}

void WebApp::establishConnection()
{
    if ( _authState == AuthSuccessful )
    {
        log_warning << TAG << "connection is already established!" << std::endl;
        return;
    }

    _userID        = "";
    _webAppVersion = "";
    _authState     = AuthNoConnection;

    QString username = settings::AppSettings::get()->readSettingsValue( M4E_SETTINGS_CAT_USER, M4E_SETTINGS_KEY_USER_LOGIN, "" );
    QString passwd   = settings::AppSettings::get()->readSettingsValue( M4E_SETTINGS_CAT_USER, M4E_SETTINGS_KEY_USER_PW, "" );
    QString server   = settings::AppSettings::get()->readSettingsValue( M4E_SETTINGS_CAT_SRV, M4E_SETTINGS_KEY_SRV_URL, "" );

    if ( !username.isEmpty() && !passwd.isEmpty() )
    {
        _authState = AuthConnecting;
        m4e::user::UserAuthentication* p_user = getOrCreateUserAuth();

        // just for the case that the server url was changed
        p_user->setServerURL( server );
        getOrCreateAppInfo()->setServerURL( server );
        getOrCreateEvent()->setServerURL( server );
        getOrCreateConnection()->setServerURL( server );
        getOrCreateUser()->setServerURL( server );
        getOrCreateMailBox()->setServerURL( server );

        p_user->requestSignIn( username, passwd );
        username.clear();
        passwd.clear();
    }
}

void WebApp::shutdownConnection()
{
    if ( _authState != AuthSuccessful )
    {
        log_warning << TAG << "there is no connection to shutdown!" << std::endl;
        return;
    }
    // first close the real-time communication connection
    getOrCreateConnection()->closeConnection();
    // now sign-off
    getOrCreateUserAuth()->requestSignOut();
    // remove any cookies
    Meet4EatRESTOperations::resetCookie();

    resetAllResources();
}

const QString& WebApp::getWebAppVersion() const
{
    return _webAppVersion;
}

WebApp::AuthState WebApp::getAuthState() const
{
    return _authState;
}

const QString& WebApp::getAuthFailReason() const
{
    return _authFailReason;
}

user::User* WebApp::getUser()
{
    return _p_user;
}

void WebApp::requestAuthState()
{
    getOrCreateUserAuth()->requestAuthState();
}

void WebApp::requestUserData()
{
    if ( _userID.isEmpty() )
    {
        log_error << TAG << "cannot request for user data, invalid user ID" << std::endl;
        return;
    }
    user::User* p_user = getOrCreateUser();
    p_user->requestUserData( _userID );
}

comm::Connection* WebApp::getConnection()
{
    return getOrCreateConnection();
}

notify::Notifications* WebApp::getNotifications()
{
    return getOrCreateNotifications();
}

event::Events* WebApp::getEvents()
{
    return getOrCreateEvent();
}

mailbox::MailBox* WebApp::getMailBox()
{
    return getOrCreateMailBox();
}

chat::ChatSystem* WebApp::getChatSystem()
{
    return getOrCreateChatSystem();
}

void WebApp::requestDocument( const QString& id, const QString& eTag )
{
    // document arrives with signal 'onCacheDocumentReady'
    getOrCreateDocumentCache()->requestDocument( id, eTag );
}

void WebApp::requestUserSearch( const QString& keyword )
{
    getOrCreateUser()->requestUserSearch( keyword );
}

void WebApp::onRESTAppInfo( QString version )
{
    log_info << TAG << "web app version: " << version << std::endl;
    _webAppVersion = version;
}

void WebApp::onResponseAuthState( bool authenticated, QString /*userId*/ )
{
    _authState = authenticated ? AuthSuccessful : AuthNoConnection;
    emit onAuthState( authenticated );
}

template< class T >
void WebApp::setupServerURL( T* p_inst ) const
{
    QString server = settings::AppSettings::get()->readSettingsValue( M4E_SETTINGS_CAT_SRV, M4E_SETTINGS_KEY_SRV_URL, "" );
    p_inst->setServerURL( server );
}

RESTAppInfo* WebApp::getOrCreateAppInfo()
{
    if ( !_p_restAppInfo )
    {
        _p_restAppInfo = new RESTAppInfo( this );
        connect( _p_restAppInfo, SIGNAL( onRESTAppInfo( QString ) ), this, SLOT( onRESTAppInfo( QString ) ) );

        setupServerURL( _p_restAppInfo );
    }
    return _p_restAppInfo;
}

user::UserAuthentication* WebApp::getOrCreateUserAuth()
{
    if ( !_p_userAuth )
    {
        _p_userAuth = new user::UserAuthentication( this );

        connect( _p_userAuth, SIGNAL( onResponseAuthState( bool, QString ) ), this, SLOT( onResponseAuthState( bool, QString ) ) );

        connect( _p_userAuth, SIGNAL( onResponseSignInResult( bool, QString, enum m4e::user::UserAuthentication::AuthResultsCode, QString ) ),
                 this, SLOT( onResponseSignInResult( bool, QString, enum m4e::user::UserAuthentication::AuthResultsCode, QString ) ) );

        connect( _p_userAuth, SIGNAL( onResponseSignOutResult( bool, enum m4e::user::UserAuthentication::AuthResultsCode, QString ) ),
                 this, SLOT( onResponseSignOutResult( bool, enum m4e::user::UserAuthentication::AuthResultsCode, QString ) ) );


        setupServerURL( _p_userAuth );
    }
    return _p_userAuth;
}

comm::Connection* WebApp::getOrCreateConnection()
{
    if ( !_p_connection )
    {
        _p_connection = new comm::Connection( this );

        connect( _p_connection, SIGNAL( onClosedConnection() ), this, SLOT( onClosedConnection() ) );

        setupServerURL( _p_connection );
    }
    return _p_connection;
}

notify::Notifications* WebApp::getOrCreateNotifications()
{
    if ( !_p_notifications )
    {
        _p_notifications = new notify::Notifications( this, this );
    }
    return _p_notifications;
}

user::User* WebApp::getOrCreateUser()
{
    if ( !_p_user )
    {
        _p_user = new user::User( this );
        connect( _p_user, SIGNAL( onResponseUserData( bool, m4e::user::ModelUserPtr ) ),
                 this, SLOT( onResponseUserData( bool, m4e::user::ModelUserPtr ) ) );

        connect( _p_user, SIGNAL( onResponseUserSearch( bool, QList< m4e::user::ModelUserInfoPtr > ) ),
                 this, SLOT( onResponseUserSearch( bool, QList< m4e::user::ModelUserInfoPtr > ) ) );

        setupServerURL( _p_user );
    }
    return _p_user;
}

event::Events* WebApp::getOrCreateEvent()
{
    if ( !_p_events )
    {
        _p_events = new event::Events( this );
        setupServerURL( _p_events );
    }
    return _p_events;
}

doc::DocumentCache* WebApp::getOrCreateDocumentCache()
{
    if ( !_p_documentCache )
    {
        _p_documentCache = new doc::DocumentCache( this );
        connect( _p_documentCache, SIGNAL( onDocumentReady( m4e::doc::ModelDocumentPtr ) ),
                 this, SLOT( onCacheDocumentReady( m4e::doc::ModelDocumentPtr ) ) );
    }
    return _p_documentCache;
}

mailbox::MailBox* WebApp::getOrCreateMailBox()
{
    if ( !_p_mailBox )
    {
        _p_mailBox = new mailbox::MailBox( this );
        setupServerURL( _p_mailBox );
    }
    return _p_mailBox;
}

chat::ChatSystem* WebApp::getOrCreateChatSystem()
{
    if ( !_p_chatSystem )
        _p_chatSystem = new chat::ChatSystem( this, this );

    return _p_chatSystem;
}

void WebApp::resetAllResources()
{
    _userID         = "";
    _authFailReason = "";
    _authState      = AuthNoConnection;

    delete _p_restAppInfo;
    _p_restAppInfo = nullptr;
    delete _p_connection;
    _p_connection = nullptr;
    delete _p_user;
    _p_user = nullptr;
    delete _p_events;
    _p_events = nullptr;
    delete _p_connection;
    _p_connection = nullptr;
    delete _p_notifications;
    _p_notifications = nullptr;
    delete _p_mailBox;
    _p_mailBox = nullptr;
    delete _p_chatSystem;
    _p_chatSystem = nullptr;
}

void WebApp::onResponseSignInResult( bool success, QString userId, m4e::user::UserAuthentication::AuthResultsCode code, QString reason )
{
    if ( success )
    {
        log_verbose << TAG << "successfully signed in user" << std::endl;
        _userID = userId;
        emit onUserSignedIn( true, userId );
        user::User* p_user = getOrCreateUser();

        p_user->requestUserData( userId );

        // after a successful sign-in start the real-time communication
        getOrCreateConnection()->connectServer();
    }
    else
    {
        _authState = AuthFail;
        _authFailReason = reason;
        emit onUserSignedIn( false, "" );
        log_verbose << TAG << "failed to sign in user (" << QString::number( code ) << "), reason: " << reason << std::endl;
    }
}

void WebApp::onResponseSignOutResult( bool success, user::UserAuthentication::AuthResultsCode code, QString reason )
{
    log_verbose << TAG << "user was signed off (" << QString::number( code ) << "), reason: " << reason << std::endl;
    emit onUserSignedOff( success );
}

void WebApp::onResponseUserData( bool success, m4e::user::ModelUserPtr user )
{
    _authState = success ? AuthSuccessful : AuthFail;
    _authFailReason = success ? "" : QApplication::translate( "WebApp", "Could not retrieve user data." );
    if ( !success )
    {
        log_warning << TAG << "could not get user data!" << std::endl;
    }

    emit onUserDataReady( user );
}

void WebApp::onCacheDocumentReady( m4e::doc::ModelDocumentPtr document )
{
    emit onDocumentReady( document );
}

void WebApp::onResponseUserSearch( bool success, QList< m4e::user::ModelUserInfoPtr > users )
{
    if ( success )
    {
        emit onUserSearch( users );
    }
    else
    {
        log_warning << TAG << "could not get user search results!" << std::endl;
        emit onUserSearch( QList< user::ModelUserInfoPtr >() );
    }
}

void WebApp::onClosedConnection()
{
    log_debug << TAG << "closed server connection" << std::endl;
    emit onServerConnectionClosed();
    _authState = AuthNoConnection;
}

} // namespace webapp
} // namespace m4e
