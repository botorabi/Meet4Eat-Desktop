/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#ifndef RESP_USER_H
#define RESP_USER_H

#include <configuration.h>
#include <webapp/m4e-api/m4e-response.h>
#include <QJsonDocument>


namespace m4e
{
namespace webapp
{

class RESTUser;

/**
 * @brief Response handler for GetUserData
 *
 * @author boto
 * @date Sep 12, 2017
 */
class ResponseGetUserData: public Meet4EatRESTResponse
{
    /**
     * @brief TAG Used for logging
     */
    const std::string TAG = "(ResponseGetUserData) ";

    public:

        explicit    ResponseGetUserData( RESTUser* p_requester );

        void        onRESTResponseSuccess( const QJsonDocument& results );

        void        onRESTResponseError( const QString& reason );

    protected:

        RESTUser*   _p_requester;
};

/**
 * @brief Response handler for GetUserSearch
 *
 * @author boto
 * @date Sep 25, 2017
 */
class ResponseGetUserSearch: public Meet4EatRESTResponse
{
    /**
     * @brief TAG Used for logging
     */
    const std::string TAG = "(ResponseGetUserSearch) ";

    public:

        explicit    ResponseGetUserSearch( RESTUser* p_requester );

        void        onRESTResponseSuccess( const QJsonDocument& results );

        void        onRESTResponseError( const QString& reason );

    protected:

        RESTUser*   _p_requester;
};

/**
 * @brief Response handler for UpdateUserData
 *
 * @author boto
 * @date No 19, 2017
 */
class ResponseUpdateUserData: public Meet4EatRESTResponse
{
    /**
     * @brief TAG Used for logging
     */
    const std::string TAG = "(ResponseUpdateUserData) ";

    public:

        explicit    ResponseUpdateUserData( RESTUser* p_requester );

        void        onRESTResponseSuccess( const QJsonDocument& results );

        void        onRESTResponseError( const QString& reason );

    protected:

        RESTUser*   _p_requester;
};

} // namespace webapp
} // namespace m4e

#endif // RESP_USER_H
