#include <iostream>
#include <sstream>
#include <stdexcept>

#include <unistd.h>
#include <sys/select.h>

#include "socket_wrapper.h"

using std::clog;
using std::ostringstream;

namespace Netrounds { namespace Util {

bool SWR_FD_ISSET(const SocketWrapper& swr, fd_set* fds)
{
    int tmpfd;
    bool retval;

    if ( (tmpfd = swr.get()) >= 0 ) {
        retval = FD_ISSET(tmpfd, fds);
    }
    else {
        retval = false;
    }

    return retval;
}

SocketWrapper::SocketWrapper(int sock) : sock_(sock)
{
}

// The move constructor
SocketWrapper::SocketWrapper(SocketWrapper&& rhs) : sock_(rhs.sock_)
{
    rhs.sock_ = -1; // Pilfered the socket from rhs, we now own sock_.
}

bool SocketWrapper::operator<(const SocketWrapper& rhs) const
{
    return sock_ < rhs.sock_;
}

SocketWrapper::~SocketWrapper()
{
    close();
}

void SocketWrapper::close()
{
    if (sock_ >= 0) {
//        clog << "--- Closing sock " << sock_ << '\n';
        ::close(sock_);
        sock_ = -1;
    }
}

void SocketWrapper::set(int sock)
{
    if (sock < 0) {
        ostringstream ss;
        ss << "Tried to set wrapped descriptor to negative value: " << sock;
        throw std::runtime_error(ss.str());
    }
    if (sock_ == sock) {
        clog << "Error, tried to re-set socket wrapper object "
            "to same desciptor ID!\n";
        return;
    }
    if (sock_ >= 0) {
//        clog << "--- Resetting wrapped sock " << sock_ << " to " << sock << '\n';
    }
    close();
    sock_ = sock;
}

int SocketWrapper::get() const
{
    return sock_;
}

}}
