#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "ip.h"
#include "config.h"
#include "util.h"
#include "log.h"

/*
 * Add an interface to your router. 
 */
void addInterface(int interface) {
    // TODO: Implement this
}

/*
 * Mark the given interface (which must have been previously added) as the
 * external interface and assign it the given ip address.
 */
void markInterfaceExternal(int interface, uint32_t ip) {
    // TODO: Implement this
}

/*
 * Add a forwarding entry to your forwarding table.  The network address is
 * given by the network and netlength arguments.  Datagrams destined for
 * this network are to be sent out on the indicated interface.
 */
void addForwardEntry(uint32_t network, int netlength, int interface) {
    // TODO: Implement this
}

/*
 * Add a port-forwarding rule to your NAT table.  Incoming datagrams using
 * the given protocol that are destined for the given natport on the
 * external interface are to be forwarded to the given internalip and
 * internal port given by the network and netlength arguments.  
 */
void addPortForwardingRule(int protocol, int natport,
                                       uint32_t internalip, int internalport) {
    // TODO: Implement this
}
void run() {
    int port = getDefaultPort();
    int fd = udp_open(port);
    // TODO: Implement this
}

int main(int argc, char **argv) {
    logConfig("router", "packet,error,failure");

    char *configFileName = "router.config";
    if (argc > 1) {
	configFileName = argv[1];
    }
    configLoad(configFileName, addInterface, markInterfaceExternal,
               addForwardEntry, addPortForwardingRule);
    run();
    return 0;
}
