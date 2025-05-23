#include <libmisc/nonblock.h>
#include <stdlib.h>
#include <string.h>

Pollio pollio_register(int nbfd, enum MiscEvent event, size_t count)
{
    return (Pollio){
        .poller = {
            .fd = nbfd,
            .events = event,
        },

        .buffer = nullptr,
        .count = event == MISCNB_EVREAD ? 0 : count,
        .complete = false,
    };
}

static void __pollio_duplicate(struct pollfd *polls, Pollio *pollios, size_t count)
{
    while (count--)
        *(polls++) = (pollios++)->poller;
}

void pollio_multiplex(Pollio *polls, size_t count, int timeout)
{
    if (!polls || count == 0)
        return;

    struct pollfd *realpolls = calloc(count, sizeof *realpolls);
    if (!realpolls)
        return;

    __pollio_duplicate(realpolls, polls, count);

    while (true) {
        int status = poll(realpolls, count, timeout);
        if (status == 0 || status == -1)
            break;

        for (size_t i = 0, complete = 0; i < count && complete != count; i++) {
            if (realpolls[i].complete) {
                complete++;
                goto nextfor;
            }


            if (realpolls[i].revents & realpolls[i].events) {
                switch (realpolls[i].events) {
                case MISCNB_EVWRITE:
                    if (realpolls[i].count <= MISCNB_PARTIAL)
                        write(realpolls[i].poller.fd, realpolls[i].buffer, realpolls[i].count);
                    else {
                        size_t written = 0;
                        size_t wrcount = realpolls[i].count;

                        while (written < wrcount) {
                            size_t remains = wrcount - written;
                            size_t wrsize = remains < MISCNB_PARTIAL ? remains : MISCNB_PARTIAL;

                            if (write(realpolls[i].poller.fd, realpolls[i].buffer + written, wrsize) == -1) {
                                realpolls[i].complete = true;
                                goto done;
                            }

                            written += wrsize;
                        }
                    }
                
                case MISCNB_EVREAD:
                    if (realpolls[i].count <= MISCNB_PARTIAL)
                        read(realpolls[i].poller.fd, realpolls[i].buffer, realpolls[i].count);
                    else {
                        size_t readed = 0;
                        size_t rdcount = realpolls[i].count;

                        while (readed < rdcount) {
                            size_t remains = wrcount - readed;
                            size_t rdsize = remains < MISCNB_PARTIAL ? remains : MISCNB_PARTIAL;

                            if (read(realpolls[i].poller.fd, realpolls[i].buffer + readed, wrsize) <= 0) {
                                realpolls[i].complete = true;
                                goto done;
                            }

                            readed += rdsize;
                        }
                    }
                }
            }

        done:
        }
    nextfor:
    }

    free(realpolls);
}

static ssize_t __ionb(int       nbfd,
                        void    *buf,
                        size_t  count,
                        int     event,
                        int     timeout)
{
    struct pollfd poller = {
        .fd = nbfd,
        .events = event,
    };

    ssize_t status;

    if (count == 0)
        return 0;

    while (true) {
        if ((status = poll(&poller, 1, timeout)) == 0)
            return status;
        else if (status == -1)
            return status;

        switch (event) {
        case POLLRDNORM:
            if (poller.revents & event) {
                status = read(nbfd, buf, count);
            }
            
            goto endpoll;

        case POLLWRNORM:
            if (poller.revents & event) {
                status = write(nbfd, buf, count);
            }
            
            goto endpoll;

        default:
            // event not supported
            return -1;
        }

    }

endpoll:
    return status;
}

ssize_t readnb(int      nbfd,
                void    *buf,
                size_t  count,
                int     timeout)
{
    return __ionb(nbfd, buf, count, POLLRDNORM, timeout);
}

void *readnball(int nbfd, int timeout)
{
    off64_t length = lseek64(nbfd, 0, SEEK_END);

    if (length == -1)
        return nullptr;
    else
        if (lseek64(nbfd, 0, SEEK_SET) == -1)
            return nullptr;

    char *buff = malloc(length);
    if (!buff)
        return nullptr;

    if (length <= MISCNB_PARTIAL)
        readnb(nbfd, buff, length, timeout);
    else {
        size_t count = 0;
        while (readnb(nbfd, buff + count, MISCNB_PARTIAL, timeout) > 0)
            count += MISCNB_PARTIAL;

        size_t len = strlen(buff) + 1;
        if (count > len) {
            char *tmp = realloc(buff, len);
            if (!tmp) {
                return buff;
            }

            buff = tmp;
        }
    }

    return buff;
}

ssize_t writenb(int     nbfd,
                void    *buf,
                size_t  count,
                int     timeout)
{
    return __ionb(nbfd, buf, count, POLLWRNORM, timeout);
}
