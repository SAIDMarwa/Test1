
import inet.applications.udpapp.UdpBasicApp;
import inet.node.inet.Router;
import inet.node.internetcloud.InternetCloud;
import flora.LoRaPhy.LoRaMedium;
import flora.LoraNode.LoRaNode;
import flora.LoraNode.LoRaGW;
import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.environment.common.PhysicalEnvironment;
import inet.environment.contract.IObjectCache;
@license(LGPL);
//
// TODO documentation
//
network LoRaWANNetwork
{
    parameters:
        int numberOfNodes = default(10);
        int numberOfGateways = default(3);
        int networkSizeX = default(10000);
        int networkSizeY = default(10000);
        @display("bgb=networkSizeX,networkSizeY");

    submodules:
        loRaNodes[numberOfNodes]: LoRaNode {
            @display("p=309,412");
        }
        loRaGW[numberOfGateways]: LoRaGW {
            @display("p=157,238");
        }
        LoRaMedium: LoRaMedium {
            @display("p=9800,26");
        }
        networkServer: StandardHost {
            parameters:
                @display("p=490,440");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                assignDisjunctSubnetAddresses = false;
                @display("p=6180,270");
        }
        internetCloud: InternetCloud {
            @display("p=2310,440");
        }
        gwRouter[numberOfGateways]: Router {
            @display("p=2310,1340");
        }
        nsRouter: Router {
            @display("p=1370,440");

        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=4120,6560");
        }

        obstacle1: PhysicalEnvironment {
            @display("p=6412,5210;w=100,h=50,red");
        }
        obstacle2: PhysicalEnvironment {
            @display("p=8600,7600;w=200,h=100,gray");
        }
    connections:
        networkServer.ethg++ <--> Eth1G <--> nsRouter.ethg++;
        nsRouter.pppg++ <--> Eth1G <--> internetCloud.pppg++;
        for i=0..numberOfGateways-1 {
            internetCloud.pppg++ <--> Eth1G <--> gwRouter[i].pppg++;
            gwRouter[i].ethg++ <--> Eth1G <--> loRaGW[i].ethg++;
        }
}

