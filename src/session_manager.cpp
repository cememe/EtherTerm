#include "session_manager.hpp"
#include "session.hpp"

#include <algorithm>
#include <boost/bind.hpp>


SessionManager::SessionManager()
    : hasSystemDisconnected(false)
{
    std::cout << "SessionManager Created" << std::endl;
}

SessionManager::~SessionManager()
{
    std::cout << "~SessionManager" << std::endl;
    // Clear Any new Waiting connections on shutdown.
    m_new_connections.clear();
}

/**
 * @brief Notifies that a user has joined the room
 * @param participant
 */
void SessionManager::join(session_ptr session)
{
    std::cout << "Joined SessionManager" << std::endl;
    m_sessions.insert(session);
}

/**
 * @brief Notifies that a user has left the room
 * @param participant
 */
void SessionManager::leave(session_ptr session)
{
    std::cout << "Left SessionManager" << std::endl;

    // If session is connected, we need to disconnect it first
    if(session->m_connection)
    {
        if(session->m_is_connected)
        {
            // Shutdown the Connection before closing
            session->m_connection->shutdown();
            // Force no longer being connected now.
            session->m_is_connected = false;
        }
        session->m_connection->close();
    }

    // Remove the Session
    m_sessions.erase(session);

    // Mark that a system has disconnect to reset window focus
    hasSystemDisconnected = true;
}

/**
 * @brief Window Session is Killed we want to switch focus to the first available window.
 */
void SessionManager::grabNewWindowFocus()
{
    // If a system hasn't diconnected then return;
    if (!hasSystemDisconnected)
    {
        return;
    }

    // Move to the first window in the list and raise that
    // Window for focus once the current windows closes.
    auto it = m_sessions.begin();
    if(it != m_sessions.end())
    {
        std::cout << "Changing Window Focus to WindowId: "
                  << (*it)->m_window_manager->getWindowId()
                  << std::endl;

        (*it)->m_window_manager->grabWindow();
    }

    // Reset for next system detected.
    hasSystemDisconnected = false;
}

/**
 * @brief Sends message to all users in the current room.
 * @param participant
 *
void SessionManager::deliver(std::string msg)
{
    if(msg.size() == 0)
        return;

    // This isn't used at this time!
    std::cout << "Deliver Global SessionManager notices: " << msg << std::endl;
    std::for_each(m_sessions.begin(), m_sessions.end(),
                  boost::bind(&Session::deliver, _1, boost::ref(msg)));
}
*/

/**
 * @brief Retrieve Number of users connected
 * Also helpful for determinging next node number.
 * @return
 */
int SessionManager::numberOfSessions()
{
    return m_sessions.size();
}

/**
 * @brief Update, Hits Session Updates for processing Data Queue.
 * @return
 */
void SessionManager::update()
{
    // Becasue disconnects can happen in a thread, we need to double check
    // If a session was poped off in the middle of the Iteration
    std::string::size_type num_sessions = m_sessions.size();
    for(auto it = m_sessions.begin(); it != m_sessions.end(); ++it)
    {
        // Make sure handle is valid once we call it.
        if (num_sessions != m_sessions.size())
        {
            return;
        }

        // Run Session Update for Event Processing.
        (*it)->update();
    }
}

/**
 * @brief Broacaster Anchors the Sessions, shutdown all session.
 * @return
 */
void SessionManager::shutdown()
{
    // Swap to pop all enteries off the stack.
    std::set<session_ptr>().swap(m_sessions);
}
