/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#include <configuration.h>
#include "widgeteventpanel.h"
#include <core/log.h>
#include <common/guiutils.h>
#include <common/dialogmessage.h>
#include <chat/chatmessage.h>
#include <ui_widgeteventpanel.h>
#include "dialogvoteshistory.h"
#include "widgeteventitem.h"
#include "widgetlocation.h"
#include "dialogbuzz.h"
#include "votes.h"


namespace m4e
{
namespace event
{

/** Minimal time interval between two buzzes, this is used for minimize spamming */
static const int M4E_BUZZ_ANTI_SPAM_TIME = 5000;

WidgetEventPanel::WidgetEventPanel( webapp::WebApp* p_webApp, QWidget* p_parent ) :
 QWidget( p_parent ),
 _p_webApp( p_webApp )
{
    setupUI();
}

WidgetEventPanel::~WidgetEventPanel()
{
    if ( _p_ui )
        delete _p_ui;
}

void WidgetEventPanel::setupEvent( const QString& id )
{
    QList< event::ModelEventPtr > events = _p_webApp->getEvents()->getUserEvents();
    event::ModelEventPtr event;
    for ( event::ModelEventPtr ev: events )
    {
        if ( ev->getId() == id )
        {
            event = ev;
            break;
        }
    }

    if ( !event.valid() )
    {
        log_error << TAG << "could not find the event with id: " << id << std::endl;
    }
    else
    {
        _event = event;
        restoreChatMessages();
        setupLocations();
        setupWidgetHead();
        setEventMembers();
    }
}

QString WidgetEventPanel::getEventId() const
{
    if ( _event.valid() )
        return _event->getId();
    return "";
}

void WidgetEventPanel::onLocationVotingStart( m4e::event::ModelEventPtr event )
{
    if ( event != _event )
        return;

    log_verbose << TAG << "time to vote for event locations" << std::endl;

    _p_ui->widgetVotingTime->setVisible( true );
    for ( auto p_widget: _widgets )
    {
        p_widget->enableVotingUI( true );
    }

    requestCurrentLoctionVotes();
}

void WidgetEventPanel::onLocationVotingEnd( m4e::event::ModelEventPtr event )
{
    if ( event != _event )
        return;

    log_verbose << TAG << "end of to voting time reached" << std::endl;

    _p_ui->widgetVotingTime->setVisible( false );
    for ( auto p_widget: _widgets )
    {
        p_widget->enableVotingUI( false );
    }
}

void WidgetEventPanel::onLinkActivated( QString link )
{
    if ( link == "CREATE_LOCATION" )
    {
        if ( _event.valid() )
        {
            emit onCreateNewLocation( _event->getId() );
        }
        else
        {
            log_error << TAG << "cannot requesr for creating new location, invalid event!" << std::endl;
        }
    }
    else if ( link == "VOTES_HISTORY" )
    {
        if ( _event.valid() )
        {
            DialogVotesHistory* p_dlg = new DialogVotesHistory( _p_webApp, nullptr, true );
            p_dlg->setupUI( _event );
            p_dlg->exec();
        }
    }
}

void WidgetEventPanel::setupUI()
{
    _p_ui = new Ui::WidgetEventPanel;
    _p_ui->setupUi( this );
    _widgets.clear();

    // setup the buzz button activation timer
    // the buzz button is initially disabled for the sake of spam protection
    _p_ui->pushButtonBuzz->setEnabled( false );
    _p_buzzActivationTimer = new QTimer( this );
    _p_buzzActivationTimer->setSingleShot( true );
    connect( _p_buzzActivationTimer, SIGNAL( timeout() ), this, SLOT( onBuzzActivationTimer() ) );

    scheduleEnableBuzz( M4E_BUZZ_ANTI_SPAM_TIME );

    _p_ui->widgetChat->setupUI( _p_webApp );
    connect( _p_webApp->getChatSystem(), SIGNAL( onReceivedChatMessageEvent( m4e::chat::ChatMessagePtr ) ), this, SLOT( onReceivedChatMessageEvent( m4e::chat::ChatMessagePtr ) ) );
    connect( _p_ui->widgetChat, SIGNAL( onSendMessage( m4e::chat::ChatMessagePtr ) ), this, SLOT( onSendMessage( m4e::chat::ChatMessagePtr ) ) );
    connect( _p_webApp->getEvents(), SIGNAL( onResponseRemoveLocation( bool, QString, QString ) ), this, SLOT( onResponseRemoveLocation( bool, QString, QString ) ) );
    connect( _p_webApp->getEvents(), SIGNAL( onResponseGetLocationVotesByTime( bool, QList< m4e::event::ModelLocationVotesPtr > ) ), this,
                                     SLOT( onResponseGetLocationVotesByTime( bool, QList< m4e::event::ModelLocationVotesPtr > ) ) );

    connect( _p_webApp->getEvents(), SIGNAL( onLocationVotingStart( m4e::event::ModelEventPtr ) ), this, SLOT( onLocationVotingStart( m4e::event::ModelEventPtr ) ) );
    connect( _p_webApp->getEvents(), SIGNAL( onLocationVotingEnd( m4e::event::ModelEventPtr ) ), this, SLOT( onLocationVotingEnd( m4e::event::ModelEventPtr ) ) );

    connect( _p_webApp->getNotifications(), SIGNAL( onEventMessage( QString, QString, QString, m4e::notify::NotifyEventPtr ) ), this,
                                            SLOT( onEventMessage( QString, QString, QString, m4e::notify::NotifyEventPtr ) ) );

    connect( _p_webApp->getNotifications(), SIGNAL( onUserOnlineStatusChanged( QString, QString, bool ) ), this,
                                            SLOT( onUserOnlineStatusChanged( QString, QString, bool ) ) );

    connect( _p_webApp->getNotifications(), SIGNAL( onEventLocationVote( QString, QString, QString, QString, bool ) ), this,
                                            SLOT( onEventLocationVote( QString, QString, QString, QString, bool ) ) );


    QColor shadowcolor( 150, 150, 150, 110 );
    common::GuiUtils::createShadowEffect( _p_ui->widgetInfo, shadowcolor, QPoint( -3, 3 ), 3 );
    common::GuiUtils::createShadowEffect( _p_ui->widgetMembers, shadowcolor, QPoint( -3, 3 ), 3 );
    common::GuiUtils::createShadowEffect( _p_ui->widgetVotesHistory, shadowcolor, QPoint( -3, 3 ), 3 );

    _p_clientArea = _p_ui->listWidget;
    _p_clientArea->setUniformItemSizes( true );
    _p_clientArea->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _p_clientArea->setAutoScroll( true );
    _p_clientArea->setViewMode( QListView::IconMode );
    _p_clientArea->setWrapping( true );
    _p_clientArea->setSpacing( 10 );

    common::GuiUtils::adaptLookAndFeel( this );
}

void WidgetEventPanel::scheduleEnableBuzz( int msec )
{
    if ( msec <= 0 )
    {
        _p_ui->pushButtonBuzz->setEnabled( true );
        _p_ui->pushButtonBuzz->setToolTip( QApplication::translate( "WidgetEventPanel", "Buzz Your Friends!" ) );
    }
    else
    {
        _p_ui->pushButtonBuzz->setEnabled( false );
        _p_ui->pushButtonBuzz->setToolTip( QApplication::translate( "WidgetEventPanel", "Buzz Cooldown..." ) );
        _p_buzzActivationTimer->start( msec );
    }
}

void WidgetEventPanel::setupWidgetHead()
{
    _p_ui->labelInfoHead->setText( _event->getName() );
    QString info;
    info += QApplication::translate( "WidgetEventPanel", "Owner: " ) + _event->getOwner()->getName();
    if ( _event->isRepeated() )
    {
        unsigned int days = _event->getRepeatWeekDays();
        QString weekdays;
        weekdays += ( days & event::ModelEvent::WeekDayMonday )    != 0 ? QApplication::translate( "WidgetEventPanel", "Mon" ) + " " : "";
        weekdays += ( days & event::ModelEvent::WeekDayTuesday )   != 0 ? QApplication::translate( "WidgetEventPanel", "Tue" ) + " " : "";
        weekdays += ( days & event::ModelEvent::WeekDayWednesday ) != 0 ? QApplication::translate( "WidgetEventPanel", "Wed" ) + " " : "";
        weekdays += ( days & event::ModelEvent::WeekDayThursday )  != 0 ? QApplication::translate( "WidgetEventPanel", "Thu" ) + " " : "";
        weekdays += ( days & event::ModelEvent::WeekDayFriday )    != 0 ? QApplication::translate( "WidgetEventPanel", "Fri" ) + " " : "";
        weekdays += ( days & event::ModelEvent::WeekDaySaturday )  != 0 ? QApplication::translate( "WidgetEventPanel", "Sat" ) + " " : "";
        weekdays += ( days & event::ModelEvent::WeekDaySunday )    != 0 ? QApplication::translate( "WidgetEventPanel", "Sun" ) + " " : "";
        info += "\n" + QApplication::translate( "WidgetEventPanel", "Repeated Event" );
        info += "\n * " + QApplication::translate( "WidgetEventPanel", "Week Days" ) + ": " + weekdays;
        info += "\n * " + QApplication::translate( "WidgetEventPanel", "At" ) + " " + _event->getRepeatDayTime().toString( "HH:mm" );
    }
    else
    {
        info += "\n" + QApplication::translate( "WidgetEventPanel", "Event Date" ) + ": " + _event->getStartDate().toString( "yyyy-M-dd HH:mm" );
    }
    _p_ui->labelInfoBody->setText( info );
    _p_ui->labelVotesToday->setText( "" );

    requestVotesOfDay();
}

void WidgetEventPanel::requestVotesOfDay()
{
    // get the votes of today
    _handleVotesToday = true;
    QDateTime tbeg = QDateTime::currentDateTime();
    QDateTime tend = tbeg;
    tbeg.setTime( QTime( 0, 0 ) );
    _p_webApp->getEvents()->requestGetLocationVotesByTime( _event->getId(), tbeg, tend );
}

void WidgetEventPanel::updateVotesOfDayUI( const QList< ModelLocationVotesPtr >& votes )
{
    _p_ui->labelVotesToday->setText( "" );
    Votes voteutils;
    QList< QList< ModelLocationVotesPtr > > sortedvotes = voteutils.sortByTimeAndVoteCount( votes, true, true );

    int     entries = 0;
    QString text;
    for ( const QList< ModelLocationVotesPtr >& currvote: sortedvotes )
    {
        for ( ModelLocationVotesPtr vote: currvote )
        {
            QString locname = vote->getLocationName();

            //! NOTE the location name was not available in older db structure of votes, fetch it from the event for this case.
            //! TODO remove this compat code later
            if ( locname.isEmpty() )
            {
                ModelLocationPtr loc = _event->getLocation( vote->getLocationId() );
                if ( loc.valid() )
                    locname = loc->getName();
            }

            if ( !text.isEmpty() )
                text += "<br>";

            text += " * " + locname + ": " + QString::number( vote->getUserNames().count() );

            // take only the first two entries
            if ( ++entries > 1 )
                break;
        }
    }
    if ( !text.isEmpty() )
    {
        _p_ui->labelVotesToday->setText( "<strong>Today</strong><br>" + text + "<br> * ...");
    }
}

void WidgetEventPanel::restoreChatMessages()
{
    if ( !_event.valid() )
    {
        log_error << TAG << "cannot restore the chat messages, invalid event!" << std::endl;
        return;
    }

    // restore the messages already received
    QList< chat::ChatMessagePtr > messages = _p_webApp->getChatSystem()->getEventMessages( _event->getId() );
    for ( auto msg: messages )
    {
        onReceivedChatMessageEvent( msg );
    }
}

void WidgetEventPanel::setupLocations()
{
    bool userisowner = _p_webApp->getUser()->isUserId( _event->getOwner()->getId() );
    if ( _event->getLocations().size() > 0 )
    {
        _p_ui->labelEmptyPanel->hide();
        _p_ui->labelEmptyPanelCreate->hide();
        for ( auto location: _event->getLocations() )
        {
            addLocation( _event, location, userisowner );
        }
        bool votinginprogress = _p_webApp->getEvents()->getIsVotingTime( _event->getId() );
        _p_ui->widgetVotingTime->setVisible( votinginprogress );
        if ( votinginprogress )
            requestCurrentLoctionVotes();
    }
    else
    {
        _p_ui->widgetVotingTime->setVisible( false );
        _p_ui->labelEmptyPanel->show();
        _p_ui->labelEmptyPanelCreate->setVisible( userisowner );
    }
}

void WidgetEventPanel::addLocation( event::ModelEventPtr event, event::ModelLocationPtr location, bool userIsOwner )
{
    WidgetLocation* p_widget = new WidgetLocation( _p_webApp, _p_clientArea );
    p_widget->setupUI( event, location, userIsOwner );
    connect( p_widget, SIGNAL( onDeleteLocation( QString ) ), this, SLOT( onDeleteLocation( QString ) ) );

    QListWidgetItem* p_item = new QListWidgetItem( _p_clientArea );
    p_item->setSizeHint( p_widget->size() );

    _p_clientArea->setItemWidget( p_item, p_widget );
    p_item->setData( Qt::UserRole, location->getId() );

    //! NOTE it seems that this is really needed after every item insertion, otherwise the items get draggable
    _p_clientArea->setDragDropMode( QListWidget::NoDragDrop );

    _widgets.append( p_widget );
}

QListWidgetItem* WidgetEventPanel::findLocationItem( const QString& locationId )
{
    for ( int i = 0; i < _p_clientArea->count(); i++ )
    {
        QListWidgetItem* p_item = _p_clientArea->item( i );
        QString locid = p_item->data( Qt::UserRole ).toString();
        if ( locid == locationId )
        {
            return p_item;
        }
    }
    return nullptr;
}

void WidgetEventPanel::setEventMembers()
{
    auto eventmembers = _event->getMembers();
    eventmembers.append( _event->getOwner() );
    _p_ui->widgetChat->setMembers( eventmembers );

    // set the members lable
    QString members;
    for ( auto member: _event->getMembers() )
    {
        if ( !members.isEmpty() )
            members += ", ";
        members += member->getName();
    }
    // limit the string
    const int MAX_MEMSTR_LEN = 120;
    if ( members.length() > MAX_MEMSTR_LEN )
    {
        members = members.mid( 0, MAX_MEMSTR_LEN - 3 );
        members += "...";
    }
    _p_ui->labelMembersBody->setText( members );
}

bool WidgetEventPanel::requestCurrentLoctionVotes()
{
    if ( !_event.valid() )
    {
        log_error << TAG << "cannot request for event location votes, invalid event" << std::endl;
        return false;
    }

    QDateTime tend, tbeg;
    if ( !_p_webApp->getEvents()->getVotingTimeWindow( _event->getId(), tbeg, tend ) )
    {
        return false;
    }

    _handleVotesByTime = true;
    _p_webApp->getEvents()->requestGetLocationVotesByTime( _event->getId(), tbeg, tend );
    return true;
}

WidgetLocation* WidgetEventPanel::findWidgetLocation( const QString& locationId )
{
    for ( auto p_widget: _widgets )
    {
        if ( p_widget->getId() == locationId )
            return p_widget;
    }
    return nullptr;
}

void WidgetEventPanel::onBtnBuzzClicked()
{
    DialogBuzz* p_dlg = new DialogBuzz( _p_webApp, this );
    p_dlg->setupUI( _event );
    if ( p_dlg->exec() == DialogBuzz::BtnSend )
        scheduleEnableBuzz( M4E_BUZZ_ANTI_SPAM_TIME );

    delete p_dlg;
}

void WidgetEventPanel::onDeleteLocation( QString id )
{
    _p_webApp->getEvents()->requestRemoveLocation( _event->getId(), id );
}

void WidgetEventPanel::onSendMessage( m4e::chat::ChatMessagePtr msg )
{
    msg->setReceiverId( _event->getId() );
    _p_webApp->getChatSystem()->sendToEventMembers( msg );
}

void WidgetEventPanel::onReceivedChatMessageEvent( chat::ChatMessagePtr msg )
{
    if ( msg->getReceiverId() == _event->getId() )
        _p_ui->widgetChat->appendChatText( msg );
}

void WidgetEventPanel::onEventMessage( QString /*senderId*/, QString senderName, QString eventId, notify::NotifyEventPtr notify )
{
    chat::ChatMessagePtr msg = new chat::ChatMessage();
    msg->setReceiverId( eventId );
    msg->setText( "(" + notify->getSubject() + ") " + notify->getText() );
    msg->setTime( QDateTime::currentDateTime() );
    msg->setSender( senderName );

    _p_webApp->getChatSystem()->addChatMessage( msg  );

    if ( eventId == _event->getId() )
        _p_ui->widgetChat->appendChatText( msg );
}

void WidgetEventPanel::onUserOnlineStatusChanged( QString senderId, QString /*senderName*/, bool online )
{
    _p_ui->widgetChat->updateUserStatus( senderId, online );
}

void WidgetEventPanel::onResponseRemoveLocation( bool success, QString eventId, QString locationId )
{
    QString text;
    if ( !success )
    {
        text = QApplication::translate( "WidgetEvent", "Could not remove location.\nReason: " ) + _p_webApp->getEvents()->getLastError();
        log_debug << TAG << "event removal failed: " + eventId << "/" << locationId << std::endl;
    }
    else
    {
        text = QApplication::translate( "WidgetEvent", "Event location was successfully removed" );
        log_debug << TAG << "event successfully removed: " + eventId << "/" << locationId << std::endl;

        auto* p_item = findLocationItem( locationId );
        if ( p_item )
        {
            delete p_item;
        }

        if ( !_event->removeLocation( locationId ) )
        {
            log_warning << TAG << "could not remove location, it was not found in event" << std::endl;
        }
    }

    common::DialogMessage msg( nullptr, false );
    msg.setupUI( QApplication::translate( "WidgetEvent", "Remove Location" ),
                 text,
                 common::DialogMessage::BtnOk );

    msg.exec();
}

void WidgetEventPanel::onResponseGetLocationVotesByTime( bool success, QList< ModelLocationVotesPtr > votes )
{
    if ( !success )
    {
        QString err = _p_webApp->getEvents()->getLastError();
        log_warning << TAG << "problem occured while retrieving the event location votes: " << err << std::endl;
    }
    else
    {
        // check if this is the reponse of votes of the day
        if ( _handleVotesToday )
        {
            _handleVotesToday = false;
            updateVotesOfDayUI( votes );
        }
        if ( _handleVotesByTime )
        {
            _handleVotesByTime = false;
            for ( ModelLocationVotesPtr v: votes )
            {
                WidgetLocation* p_widget = findWidgetLocation( v->getLocationId() );
                if ( p_widget )
                {
                    p_widget->updateVotes( v );
                }
            }
        }
    }
}

void WidgetEventPanel::onEventLocationVote( QString senderId, QString /*senderName*/, QString /*eventId*/, QString locationId, bool /*vote*/ )
{
    // suppress echo
    QString userid = _p_webApp->getUser()->getUserData()->getId();
    if ( senderId == userid )
        return;

    WidgetLocation* p_widget = findWidgetLocation( locationId );
    if ( !p_widget )
        return;

    if ( p_widget->getVotes().valid() )
    {
        _p_webApp->getEvents()->requestGetLocationVotesById( p_widget->getVotes()->getId() );
        requestVotesOfDay();
    }
    else
    {
        _handleVotesToday = true;
        requestCurrentLoctionVotes();
    }
}

void WidgetEventPanel::onBuzzActivationTimer()
{
    _p_ui->pushButtonBuzz->setEnabled( true );
    _p_ui->pushButtonBuzz->setToolTip( QApplication::translate( "WidgetEventPanel", "Buzz Your Friends!" ) );
}

} // namespace event
} // namespace m4e
