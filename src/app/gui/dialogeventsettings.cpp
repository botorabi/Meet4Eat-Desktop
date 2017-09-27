/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#include "dialogeventsettings.h"
#include <core/log.h>
#include "guiutils.h"
#include <ui_widgeteventsettings.h>
#include <QTableWidgetItem>
#include <assert.h>


namespace m4e
{
namespace ui
{

DialogEventSettings::DialogEventSettings( data::WebApp* p_webApp, QWidget* p_parent ) :
 BaseDialog( p_parent ),
 _p_webApp( p_webApp )
{
    _p_ui = new Ui::WidgetEventSettings();
}

DialogEventSettings::~DialogEventSettings()
{
    delete _p_ui;
}

void DialogEventSettings::setupUI( data::ModelEventPtr event )
{
    _event = event;
    decorate( *_p_ui );

    connect( _p_webApp, SIGNAL( onDocumentReady( m4e::data::ModelDocumentPtr ) ), this, SLOT( onDocumentReady( m4e::data::ModelDocumentPtr ) ) );
    connect( _p_webApp, SIGNAL( onUserSearch( QList< m4e::data::ModelUserInfoPtr > ) ), this, SLOT( onUserSearch( QList< m4e::data::ModelUserInfoPtr > ) ) );
    connect( _p_ui->pushButtonAddMember, SIGNAL( clicked() ), this, SLOT( onBtnAddMemberClicked() ) );
    connect( _p_ui->lineEditSearchMember, SIGNAL( returnPressed() ), this, SLOT( onLineEditSeachtReturnPressed() ) );

    setTitle( QApplication::translate( "DialogEventSettings", "Event Settings" ) );
    QString applybtn( QApplication::translate( "DialogEventSettings", "Apply" ) );
    QString cancelbtn( QApplication::translate( "DialogEventSettings", "Cancel" ) );
    setupButtons( &applybtn, &cancelbtn, nullptr );
    setResizable( true );

    _p_ui->lineEditName->setText( event->getName() );
    _p_ui->textEditDescription->setPlainText( event->getDescription() );
    _p_ui->checkBoxIsPublic->setChecked( event->getIsPublic() );
    _p_ui->dateTimeEditStart->setDateTime( event->getStartDate() );
    _p_ui->timeEditDayTime->setTime( event->getRepeatDayTime() );

    setupMembers( event );
    setupWeekDays( _event->getRepeatWeekDays() );

    // load  the image only if a valid photo id exits
    QString photoid = event->getPhotoId();
    if ( !photoid.isEmpty() && ( photoid != "0" ) )
    {
        _p_webApp->requestDocument( photoid, event->getPhotoETag() );
    }
}

void DialogEventSettings::onDocumentReady( m4e::data::ModelDocumentPtr document )
{
    if ( !document.valid() )
        return;

    QString photoid = _event->getPhotoId();
    if ( !photoid.isEmpty() && ( document->getId() == photoid ) )
    {
        _p_ui->labelPhoto->setPixmap( GuiUtils::createRoundIcon( document ) );
    }

    // check if a member photo arrived
    if ( !_memberPhotos.contains( document->getId() ) )
        return;

    int row = _memberPhotos.value( document->getId() );
    QPixmap pix = GuiUtils::createRoundIcon( document );
    QTableWidgetItem* p_item = _p_ui->tableWidgetMembers->item( row, 0 );
    p_item->setIcon( QIcon( pix ) );
}

void DialogEventSettings::onUserSearch( QList< data::ModelUserInfoPtr > users )
{
    _p_ui->comboBoxSearchMember->clear();
    for ( auto user: users )
    {
        _p_ui->comboBoxSearchMember->addItem( user->getName(), user->getId() );
    }
}

void DialogEventSettings::onMemberRemoveClicked()
{
    QPushButton* p_btn = dynamic_cast< QPushButton* >( sender() );
    assert ( p_btn && "unexpected event sender, a button was expected!" );

    QString memberId = p_btn->property( "userId" ).toString();
    log_verbose << TAG << "TODO remove member: " << memberId.toStdString() << std::endl;
}

void DialogEventSettings::onBtnAddMemberClicked()
{
    int index = _p_ui->comboBoxSearchMember->currentIndex();
    if ( index < 0 )
        return;

    QString id = _p_ui->comboBoxSearchMember->itemData( index ).toString();

    log_verbose << TAG << "TODO onBtnAddMemberClicked, adding member: " << id.toStdString() << std::endl;

#if 0
    QList< data::ModelUserInfoPtr > members = _event->getMembers();
    data::ModelUserInfoPtr newmember = new data::ModelUserInfo();
    newmember->setId( id );
    newmember->setName( _p_ui->comboBoxSearchMember->itemText( index ) );
//    newmember->setPhotoId( ??? );
//    newmember->setPhotoETag( ??? );
    members.append( newmember );

    //! TODO - check if the member is already part of the event
    //!      - request the server for adding the member

    _event->setMembers( members );
    setupMembers( _event );
#endif
}

void DialogEventSettings::onLineEditSeachtReturnPressed()
{
    log_verbose << TAG << "onLineEditSeachtReturnPressed" << std::endl;

    QString keyword = _p_ui->lineEditSearchMember->text();
    keyword = keyword.trimmed();

    if ( keyword.length() < 3 )
        return;

    if ( keyword.length() > 32 )
        keyword = keyword.mid( 0, 32 );

    _p_webApp->requestUserSearch( keyword );
}

void DialogEventSettings::setupWeekDays( unsigned int weekDays )
{
    _p_ui->pushButtonWDMon->setChecked( ( weekDays & data::ModelEvent::WeekDayMonday ) != 0 );
    _p_ui->pushButtonWDTue->setChecked( ( weekDays & data::ModelEvent::WeekDayTuesday ) != 0 );
    _p_ui->pushButtonWDWed->setChecked( ( weekDays & data::ModelEvent::WeekDayWednesday ) != 0 );
    _p_ui->pushButtonWDThu->setChecked( ( weekDays & data::ModelEvent::WeekDayThursday ) != 0 );
    _p_ui->pushButtonWDFri->setChecked( ( weekDays & data::ModelEvent::WeekDayFriday ) != 0 );
    _p_ui->pushButtonWDSat->setChecked( ( weekDays & data::ModelEvent::WeekDaySaturday ) != 0 );
    _p_ui->pushButtonWDSun->setChecked( ( weekDays & data::ModelEvent::WeekDaySunday ) != 0 );
}

void DialogEventSettings::setupMembers( data::ModelEventPtr event )
{
    QList< data::ModelUserInfoPtr > members = event->getMembers();

    _p_ui->tableWidgetMembers->clear();
    _p_ui->tableWidgetMembers->setColumnCount( 2 );
    _p_ui->tableWidgetMembers->setRowCount( members.size() );

    int row = 0;
    for ( data::ModelUserInfoPtr member: members )
    {
        QTableWidgetItem* p_item = new QTableWidgetItem( QIcon(), member->getName() );
        p_item->setFlags( p_item->flags() ^ Qt::ItemIsEditable );
        _p_ui->tableWidgetMembers->setItem( row, 0, p_item );

        if ( event->getOwner().valid() && GuiUtils::userIsOwner( event->getOwner()->getId(), _p_webApp ) )
        {
            _p_ui->tableWidgetMembers->setCellWidget( row, 1, createRemoveMemberButton( member->getId() ) );
        }

        // request the photo
        QString photoid = member->getPhotoId();
        if ( !photoid.isEmpty() && ( photoid != "0" ) )
        {
            _memberPhotos.insert( photoid, row );
            _p_webApp->requestDocument( photoid, member->getPhotoETag() );
        }
        row++;
    }

    _p_ui->tableWidgetMembers->resizeColumnsToContents();
}

QWidget *DialogEventSettings::createRemoveMemberButton( const QString& memberId )
{
    QWidget* p_widget = new QWidget( this );
    p_widget->setStyleSheet( "background: transparent;" );
    p_widget->setLayout( new QHBoxLayout( nullptr ) );
    p_widget->layout()->addItem( new QSpacerItem( 20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

    QPushButton* p_btn = new QPushButton( QIcon( ":/icon-close.png" ), "", p_widget );
    p_btn->setFocusPolicy( Qt::NoFocus );
    p_btn->setToolTip( QApplication::translate( "DialogEventSettings", "Remove member from event" ) );
    p_btn->setProperty( "userId", memberId );
    p_btn->setIconSize( QSize( 20, 20 ) );
    p_btn->setMaximumSize( QSize( 20, 20 ) );
    p_btn->setMinimumSize( QSize( 20, 20 ) );
    p_widget->layout()->addWidget( p_btn );

    connect( p_btn, SIGNAL( clicked() ), this, SLOT( onMemberRemoveClicked() ) );

    return p_widget;
}

} // namespace ui
} // namespace m4e
