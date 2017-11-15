/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#ifndef WIDGETEVENT_H
#define WIDGETEVENT_H

#include <webapp/webapp.h>
#include <event/modelevent.h>
#include <event/modellocation.h>
#include <chat/chatsystem.h>
#include <QListWidget>
#include <QWidget>
#include <QLabel>
#include <QMap>


namespace Ui
{
    class WidgetEventPanel;
}


namespace m4e
{
namespace event
{

class WidgetLocation;

/**
 * @brief Class for decorating the "Event" widget.
 *
 * @author boto
 * @date Aug 2, 2017
 */
class WidgetEventPanel : public QWidget
{
    /**
     * @brief TAG Used for logging
     */
    const std::string TAG = "(WidgetEventPanel) ";

    Q_OBJECT

    public:

        /**
         * @brief Create a new event panel widget
         *
         * @param p_webApp          Web application interface
         * @param p_parent          Parent widget
         */
                                    WidgetEventPanel( webapp::WebApp* p_webApp, QWidget* p_parent = nullptr );

        /**
         * @brief Destroy the panel.
         */
        virtual                     ~WidgetEventPanel();

        /**
         * @brief Set the event which is represented by this widget.
         *
         * @param id    Event ID
         */
        void                        setEvent( const QString& id );

        /**
         * @brief Set the ChatSystem which allows the chat messanging.
         *
         * @param p_chatSystem  The chat system
         */
        void                        setChatSystem( m4e::chat::ChatSystem* p_chatSystem );

    signals:

        /**
         * @brief This signal is emitted when the user requests for creating a new location.
         *
         * @param eventId  The ID of event which should get a new location
         */
        void                        onCreateNewLocation( QString eventId );

    protected slots:

        /**
         * @brief Called when a link in any QLabel was clicked.
         *
         * @param link Activated link
         */
        void                        onLinkActivated( QString link );

        /**
         * @brief The buzz button was clicked.
         */
        void                        onBtnBuzzClicked();

        /**
         * @brief Emitted when the user clicks "location delete" button in location widget.
         *
         * @param id   The location ID
         */
        void                        onDeleteLocation( QString id );

        /**
         * @brief The chat widget has a new message to send.
         *
         * @param msg Chat message
         */
        void                        onSendMessage( m4e::chat::ChatMessagePtr msg );

        /**
         * @brief This signal is received from chat system whenever a new event chat message arrived.
         *
         * @param msg Chat message
         */
        void                        onReceivedChatMessageEvent( m4e::chat::ChatMessagePtr msg );

        /**
         * @brief Results of remove event location request.
         *
         * @param success    true if user data could successfully be retrieved, otherwise false
         * @param eventId    ID of event
         * @param locationId ID of location to remove
         */
        void                        onResponseRemoveLocation( bool success, QString eventId, QString locationId );

        /**
         * @brief Results of location votes request by time range.
         *
         * @param success   true if user votes could successfully be retrieved, otherwise false
         * @param votes     The event location votes list
         */
        void                        onResponseGetLocationVotesByTime( bool success, QList< m4e::event::ModelLocationVotesPtr > votes );

    protected:

        /**
         * @brief Setup the widget.
         */
        void                        setupUI();

        /**
         * @brief Setup the head elements in event widget (info fields, etc.)
         */
        void                        setupWidgetHead();

        /**
         * @brief Setup all location widgets.
         */
        void                        setupLocations();

        /**
         * @brief Add a new location for an event
         *
         * @param event         The event
         * @param location      New location to add
         * @param userIsOwner   Is the user also the owner of the event?
         */
        void                        addLocation( event::ModelEventPtr event, event::ModelLocationPtr location, bool userIsOwner );

        /**
         * @brief Try to find the widget item of an event location given its ID.
         *
         * @param locationId  Location ID
         * @return Widget item, or nullptr if the ID was not found
         */
        QListWidgetItem*            findLocationItem( const QString& locationId );

        /**
         * @brief Setup the event members.
         */
        void                        setEventMembers();

        /**
         * @brief Request for currently running event location votes.
         *
         * @return Return false if it's not time to vote.
         */
        bool                        requestCurrentLoctionVotes();

        /**
         * @brief Try to find a WidgetLocation given its ID.
         *
         * @param locationId    The location ID
         * @return              The widget if the ID was found, otherwise nullptr
         */
        WidgetLocation*             findWidgetLocation( const QString& locationId );

        Ui::WidgetEventPanel*       _p_ui           = nullptr;

        QListWidget*                _p_clientArea   = nullptr;

        webapp::WebApp*             _p_webApp       = nullptr;

        m4e::chat::ChatSystem*      _p_chatSystem   = nullptr;

        typedef QMap< QString /*id*/, QString /*name*/>  Locations;

        m4e::event::ModelEventPtr   _event;

        QList< WidgetLocation* >    _widgets;
};

} // namespace event
} // namespace m4e

#endif // WIDGETEVENT_H
