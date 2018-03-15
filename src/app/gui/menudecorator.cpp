/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#include "menudecorator.h"

namespace m4e
{
namespace gui
{

MenuDecorator::MenuDecorator( QMenu* p_menu ) :
 _p_menu( p_menu )
{
}

void MenuDecorator::addMenuSeperator()
{
    QAction* p_action = new QAction();
    p_action->setSeparator( true );
    _p_menu->addAction( p_action );
}

void MenuDecorator::addMenuItem( const QString& label, const QVariant& data )
{
    QAction* p_action = new QAction( label );
    p_action->setData( data );
    _p_menu->addAction( p_action );
}

void MenuDecorator::addMenuItemCheckbox( const QString& label, const QVariant& data, bool checked )
{
    QAction* p_action = new QAction( label );
    p_action->setCheckable( true );
    p_action->setData( data );
    p_action->setChecked( checked );
    _p_menu->addAction( p_action );
}

} // namespace gui
} // namespace m4e
