/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#include <configuration.h>
#include "widgeteventlist.h"
#include <core/log.h>
#include "widgeteventitem.h"
#include <QLayout>


namespace m4e
{
namespace event
{

WidgetEventList::WidgetEventList( webapp::WebApp* p_webApp, QWidget* p_parent ) :
 QWidget( p_parent ),
 _p_webApp( p_webApp )
{
    setupUI();
}

void WidgetEventList::selectEvent( const QString& eventId )
{
    onClicked( eventId );
}

void WidgetEventList::selectFirstEvent()
{
    if ( _widgets.size() > 0 )
        onClicked( _widgets.at( 0 )->getId() );
}

void WidgetEventList::setupUI()
{
    connect( _p_webApp->getEvents(), SIGNAL( onResponseGetEvent( bool, m4e::event::ModelEventPtr ) ), this, SLOT( onResponseGetEvent( bool, m4e::event::ModelEventPtr ) ) );

    QVBoxLayout* p_layout = new QVBoxLayout();
    setLayout( p_layout );

    QList< m4e::event::ModelEventPtr > events = _p_webApp->getEvents()->getUserEvents();
    for ( auto ev: events )
    {
        addEvent( ev );
    }
}

void WidgetEventList::addEvent( m4e::event::ModelEventPtr event )
{
    WidgetEventItem* p_item = new WidgetEventItem( _p_webApp, this );
    p_item->setupUI( event );
    connect( p_item, SIGNAL( onClicked( QString ) ), this, SLOT( onClicked( QString ) ) );
    connect( p_item, SIGNAL( onRequestUpdateEvent( QString ) ), this, SLOT( onRequestUpdateEvent( QString ) ) );

    layout()->addWidget( p_item );
    _widgets.append( p_item );
}

void WidgetEventList::onClicked( QString id )
{
    for ( WidgetEventItem* p_widget: _widgets )
    {
        p_widget->setSelectionMode( p_widget->getId() != id );
    }

    // forward the signal
    emit onEventSelection( id );
}

void WidgetEventList::onRequestUpdateEvent( QString id )
{
    _p_webApp->getEvents()->requestGetEvent( id );
}

void WidgetEventList::onResponseGetEvent( bool success, m4e::event::ModelEventPtr event )
{
    if ( !success )
    {
        log_warning << TAG << "could not get event data" << std::endl;
    }
    else
    {
        // update the event widget
        for ( WidgetEventItem* p_item: _widgets )
        {
            if ( p_item->getId() == event->getId() )
            {
                p_item->updateEvent( event );
                selectEvent( event->getId() );
                break;
            }
        }
    }
}

} // namespace event
} // namespace m4e
