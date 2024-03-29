#ifndef _REMOTE_LOG_H_
#define _REMOTE_LOG_H_

#import <netinet/in.h>
#import <sys/socket.h>
#import <unistd.h>
#import <arpa/inet.h>
#import <netdb.h>
#import <ifaddrs.h> // For getifaddrs()
#import <net/if.h> // For IFF_LOOPBACK
// change this to match your destination (server) IP address
#define RLOG_IP_ADDRESS "tomslaptop.local"
#define RLOG_PORT 11909


/*
Connectivity testing code pulled from Apple's Reachability Example: https://developer.apple.com/library/content/samplecode/Reachability
 */
static bool hasConnectivity (){
    struct ifaddrs *addresses;
    struct ifaddrs *cursor;
    BOOL wiFiAvailable = NO;
    if (getifaddrs(&addresses) != 0) return NO;

    cursor = addresses;
    while (cursor != NULL) {
        if (cursor -> ifa_addr -> sa_family == AF_INET
            && !(cursor -> ifa_flags & IFF_LOOPBACK)) // Ignore the loopback address
        {
            // Check for WiFi adapter
            if (strcmp(cursor -> ifa_name, "en0") == 0) {
                wiFiAvailable = YES;
                break;
            }
        }
        cursor = cursor -> ifa_next;
    }

    freeifaddrs(addresses);
    return wiFiAvailable;
}


__attribute__((unused)) static void RLogv(NSString* format, va_list args)
{
    #if DEBUG
        NSString* str = [[NSString alloc] initWithFormat:format arguments:args];

        int sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sd <= 0)
        {
            NSLog(@"[RemoteLog] Error: Could not open socket");
            return;
        }

        int broadcastEnable = 1;
        int ret = setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
        if (ret)
        {
            NSLog(@"[RemoteLog] Error: Could not open set socket to broadcast mode");
            close(sd);
            return;
        }
        struct sockaddr_in broadcastAddr;                                                                                        
        memset(&broadcastAddr, 0, sizeof broadcastAddr);                                                                
        broadcastAddr.sin_family = AF_INET;

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;        // PF_INET if you want only IPv4 addresses
        hints.ai_protocol = IPPROTO_TCP;
        if (!(hasConnectivity()))return;
        
        struct addrinfo *addrs, *addr;
        char *ipbuf;
        ipbuf = strdup(" ");;
        getaddrinfo(RLOG_IP_ADDRESS, NULL, &hints, &addrs);
        for (addr = addrs; addr; addr = addr->ai_next) {
            char host[NI_MAXHOST];
            getnameinfo(addr->ai_addr, addr->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
            ipbuf = host;
            NSLog(@"bruh");
        }
        freeaddrinfo(addrs);


        inet_pton(AF_INET, ipbuf, &broadcastAddr.sin_addr);
        broadcastAddr.sin_port = htons(RLOG_PORT);
        char* request = (char*)[str UTF8String];
        ret = sendto(sd, request, strlen(request), 0, (struct sockaddr*)&broadcastAddr, sizeof broadcastAddr);
        if (ret < 0)
        {
            NSLog(@"[RemoteLog] Error: Could not send broadcast");
            close(sd);
            return;
        }
        close(sd);
    #endif
}

__attribute__((unused)) static void RLog(NSString* format, ...)
{
    #if DEBUG
        va_list args;
        va_start(args, format);
        RLogv(format, args);
        va_end(args);
    #endif
}
#endif
