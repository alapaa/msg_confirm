namespace Netrounds { namespace Util {

class SocketWrapper;

/**
 * @brief Helper to check if descriptor still valid before FD_ISSET.
 *
 * Name in caps since original socket macro is in caps.
 */
bool SWR_FD_ISSET(const SocketWrapper& swr, fd_set* fds);

/**
 * Class for RAII-wrapping a socket (C++-style cleanup).
 * RAII means Resource Acquisition is Initialization,
 * a better name may be Resource Release is Destruction.
 */
class SocketWrapper {
public:
    SocketWrapper(int sock = -1);
    ~SocketWrapper();

    /*
     * If necessary, close wrapped socket. Can be used to manually close. Also
     * used by destructor.
     */
    void close();

    /**
     * Set wrapped socket. If we already wrapped another socket, close that
     * first.
     */
    void set(int sock);

    /*
     * Return wrapped socket descriptor. Do not call close() manually on
     * descriptor!
     */
    int get() const;

    /*
     * Move constructor to enable use of SocketWrappers in STL containers.
     */
    SocketWrapper(SocketWrapper&& swr);

    /*
     * Comparison, for use in std::map
     */
    bool operator<(const SocketWrapper& rhs) const;

    /*
     * Exclusive ownership, no copy initialization/copy assignment
     */
    SocketWrapper(const SocketWrapper&) = delete;
    const SocketWrapper& operator=(const SocketWrapper&) = delete;

private:
    int sock_;
};

}}
