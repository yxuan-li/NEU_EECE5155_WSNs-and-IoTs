#include <stdio.h>
#include <stdlib.h>
#include <omnetpp.h>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include "MobileSink.h"

using namespace omnetpp;
using namespace std;

class MobileSinkNode2BD: public cSimpleModule
{
    private:
        cMessage *timer;
        cMessage *LRB;
        cMessage *SRB;
        cMessage *ACK;
        double Tbi;
        double v;
        double x_start;
        double y_start;
        double x_end;
        double y_end;

    public:
        MobileSinkNode2BD();
        virtual ~MobileSinkNode2BD();
    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void refreshDisplay();
};

// Mobile Sink
MobileSinkNode2BD::MobileSinkNode2BD()
{
    timer = LRB = SRB = ACK = nullptr;
}

MobileSinkNode2BD::~MobileSinkNode2BD()
{
    cancelAndDelete(timer);
    cancelAndDelete(LRB);
    cancelAndDelete(SRB);
    cancelAndDelete(ACK);
}


class WirelessChannel: public cSimpleModule
{
    private:
        cMessage *timer1;
        cMessage *timer2;
        cMessage *timer3;
        cMessage *wirelessmsg;
        int R;
        int r;
        bool packet_loss;

    public:
        WirelessChannel();
        virtual ~WirelessChannel();
    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual bool loss_probality();
};

// Wireless Channel
WirelessChannel::WirelessChannel()
{
    timer1 = timer2 = timer3 = wirelessmsg = nullptr;
}

WirelessChannel::~WirelessChannel()
{
    cancelAndDelete(timer1);
    cancelAndDelete(timer2);
    cancelAndDelete(timer3);
    cancelAndDelete(wirelessmsg);
}


class SensorNode2BD: public cSimpleModule
{
    private:
        cMessage *packet;
        cMessage *off;
        cMessage *on;
        cMessage *packet_timeout;
        cMessage *LRB_timeout;
        double Ton;
        double LDC;
        double HDC;
    public:
        SensorNode2BD();
        virtual ~SensorNode2BD();
    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
};


SensorNode2BD::SensorNode2BD()
{
    off = on = packet = packet_timeout = LRB_timeout=  nullptr;
}

SensorNode2BD::~SensorNode2BD()
{
    cancelAndDelete(off);
    cancelAndDelete(on);
    cancelAndDelete(packet);
    cancelAndDelete(packet_timeout);
    cancelAndDelete(LRB_timeout);
}

// define modules
Define_Module(MobileSinkNode2BD);
Define_Module(WirelessChannel);
Define_Module(SensorNode2BD);

// MobileSink module
void MobileSinkNode2BD::initialize(){
    timer = new cMessage("timer");
    LRB = new cMessage("LRB");
    SRB = new cMessage("SRB");

    Tbi = par("Tbi");
    v = par("v");
    velocity = v;
    passage_counter = par("counter");
    x_start = 500 + double(par("x_start"));
    y_start = 400 - double(par("y_start"));
    x_end = 500 + double(par("x_end"));
    y_end = 400 - double(par("y_end"));
    EV<<"x start: " << x_start << " x end: " <<x_end <<" y start: " << y_start<<" y end: " << y_end<<"\n";
    WATCH(x);
    WATCH(y);
    WATCH(d);
    WATCH(passage_counter);
    WATCH(packets_send);
    WATCH(discovery_time);
    WATCH(discovery_ratio);

    // Calculate coordinate components of v, dx and dy
    double xrange = x_end - x_start;
    double yrange = y_end - y_start;
    if(x_start != x_end && y_start != y_end){
        double tmp = sqrt(pow(xrange,2) + pow(yrange,2));
        cosc = xrange / tmp;
        sinc = yrange / tmp;
        v_cos = v * cosc;
        v_sin = v * sinc;
    }
    x = x_start;
    y = y_start;
    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);

    scheduleAt(simTime(), LRB);
    scheduleAt(simTime() + Tbi, SRB);
    scheduleAt(simTime(), timer);
}

void MobileSinkNode2BD::handleMessage(cMessage *msg)
{
    if(msg == timer){
//        cancelAndDelete(timer);
        delete msg;
        timer = new cMessage("timer");
        refreshDisplay();
        scheduleAt(simTime() + 0.001, timer);
    }else if (msg == LRB){
        send(msg, "out");
        LRB = new cMessage("LRB");
        scheduleAt(simTime() + 2 * Tbi, LRB);
    }else if (msg == SRB){
        send(msg, "out");
        SRB = new cMessage("SRB");
        scheduleAt(simTime() + 2 * Tbi, SRB);
    }else if (!strcmp(msg->getName(),"packet")){
        ACK = new cMessage("ACK");
        send(ACK, "out");
        delete msg;
    }
}

void MobileSinkNode2BD::refreshDisplay()
{
    if (x_start == x_end){
        y = y_start + v * (simTime().dbl() - last_end_time);
    }else if(y_start == y_end){
        x = x_start + v * (simTime().dbl() - last_end_time);
    }
    else{
        x = x_start + v_cos * (simTime().dbl() - last_end_time);
        y = y_start + v_sin * (simTime().dbl() - last_end_time);
    }
    if(x > x_end || y > y_end){
        // counter change here
        passage_counter = passage_counter - 1;
        discovery_ratio = discovery_time / double(1000 - passage_counter);
        throughput = packets_send / (1000 - passage_counter) * 133;
        if(discoveried){
            all_energy_discovery = all_energy_discovery + energy_discovery;
            all_energy_transfer  = all_energy_transfer  + energy_transfer;
            average_energy_discovery = all_energy_discovery / double(1000 - passage_counter);
            average_energy_transfer  = all_energy_transfer  / double(1000 - passage_counter);
        }
        energy_discovery = 0;
        energy_transfer  = 0;
        discoveried = false;
        if (passage_counter <= 0){
//            getSimulation()->getActiveEnvir()->alert("Simulation ends.");
            endSimulation();
        }
        x = x_start;
        y = y_start;
        last_end_time = simTime().dbl();
    }
    d = sqrt(pow((x-500),2) + pow((y - 400),2));
//    EV<<"distance "<<d;
    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);
}

// wireless module
void WirelessChannel::initialize(){
    R = par("R");
    r = par("r");
    packet_loss = par("packet_loss");
    discovery_range = R;
    communication_range = r;
}

void WirelessChannel::handleMessage(cMessage *msg)
{
    if (((d > communication_range) and ((!strcmp(msg->getName(),"SRB")) or (!strcmp(msg->getName(),"packet")) or (!strcmp(msg->getName(),"ACK")))) or ((d > discovery_range) and (!strcmp(msg->getName(),"LRB")))){
        delete msg;
        bubble("too far");
    }else if (loss_probality()){
        delete msg;
        bubble("lost");
    }else{
        int sid = msg->getArrivalGate()->getIndex();
        sid ^= 1;
        send(msg, "out", sid);
    }
}

bool WirelessChannel::loss_probality(){
    if(packet_loss){
        double random_double =  rand()%(10000) / double(10000);
        double p = d / double(4 * R);
        return random_double < p;
    }
    return false;
}

void SensorNode2BD::initialize(){
    Ton = par("Ton");

    LDC = par("LDC");
    HDC = par("HDC");
    p_rx = par("p_rx");
    p_tx = par("p_tx");
    P_duration = par("P_duration");
    waiting_LRB = 1;
    waiting_SRB = 0;
    transmitting = 0;

    WATCH(sensor_on);
    WATCH(waiting_LRB);
    WATCH(waiting_SRB);
    WATCH(transmitting);
    WATCH(ack_lost);
    WATCH(d);
    WATCH(off_time);
    WATCH(passage_counter);
    WATCH(packets_send);
    WATCH(throughput);
    WATCH(discovery_time);
    WATCH(discovery_ratio);
    WATCH(on_time);
    WATCH(discoveried);
    WATCH(average_energy_discovery);
    WATCH(average_energy_transfer);
    LRB_timeout_time = 2 * communication_range / velocity;
    WATCH(LRB_timeout_time);
    // calculate Toff time
//    Ton = Ton * 2;
    low_off_time = (1 - LDC)/LDC * Ton;
    high_off_time = (1 - HDC)/HDC * Ton;

    EV<<"Sensor low off time: "<<low_off_time<<" Sensor high off time: "<<high_off_time<<"\n";
//    Ton = Ton * 2;
    // initial off time
    off_time = low_off_time;
    ack_lost = 0;
    // random Toff time
    srand(time(0));
    double randtime = rand()%int((Ton + low_off_time) * 1000) / double(1000);
    EV<<"Seneor randtime: "<< randtime<<"\n";
    off = new cMessage("off");
    scheduleAt(simTime() + randtime, off);
//    scheduleAt(simTime(), off);
}

void SensorNode2BD::handleMessage(cMessage *msg)
{
    EV << "Sensor received: " << msg->getName() << "\n";
    EV << "Sensor received: " << msg << "\n";
    if(transmitting == 1){
        if(!strcmp(msg->getName(),"ACK")){
            delete msg;
            packets_send = packets_send + 1;
            // reset ack_lost count
            ack_lost = 0;
            cancelAndDelete(packet_timeout);
            packet_timeout = new cMessage("packet_timeout");
            scheduleAt(simTime() + 0.028001, packet_timeout);

            // energy
            energy_transfer += p_rx * (P_duration + 2 * 0.010); // ack, packet, beacon  same duration

            // send another packet
            packet = new cMessage("packet");
            send(packet,"out");
            bubble("Send packet.");
        }else if(msg == packet_timeout){
            delete msg;
            ack_lost = ack_lost + 1; // ack_lost + 1

            //energy
            energy_transfer += p_rx * 0.024001;

            if(ack_lost == 3){
                // 3 consecutive -> low cycle and turn off
                sensor_on = 0;
                waiting_LRB = 1;
                waiting_SRB = 0;
                transmitting = 0;
                ack_lost = 0;
                off_time = low_off_time;
                off = new cMessage("off");
                scheduleAt(simTime() + off_time, off);
            }else{
                // send again and reset packet_timeout
                packet_timeout = new cMessage("packet_timeout");
                scheduleAt(simTime() + 0.028001, packet_timeout);
                // energy send packet
                energy_transfer += p_tx * P_duration;
                packet = new cMessage("packet");
                send(packet,"out");
                bubble("Retransmit...");
            }
        }else{ // LRB  SRB Ton
            bubble("delete LRB/SRB/Ton");
            delete msg;
        }
    }else{ // not transmitting
        if(!strcmp(msg->getName(),"off")){
            // Toff ends
            delete msg;
            EV<<"Sensor on..."<<"\n";
            bubble("Turned on.");
            sensor_on = 1;
            on = new cMessage("on");
            scheduleAt(simTime() + Ton, on);
            // energy
            on_time = on_time + 1;
            if(!discoveried){
                energy_discovery = energy_discovery + p_rx * Ton;
                EV<<"energy_discovery "<<energy_discovery<<"\n";
                tmp_time = simTime().dbl();
            }
        }else if(!strcmp(msg->getName(),"on")){
            // Ton ends
            delete msg;
            EV<<"Sensor off..."<<"\n";
            bubble("Turned off.");
            sensor_on = 0;
            off = new cMessage("off");
            scheduleAt(simTime() + off_time, off);
        }else if(!strcmp(msg->getName(),"LRB_timeout")){
            // turn off and return to low cycle
            is_LRB_timeout = 0;
            waiting_LRB = 1;
            waiting_SRB = 0;
            transmitting = 0;
            if(sensor_on){
                cancelAndDelete(on);
            }else{
                cancelAndDelete(off);
            }
            sensor_on = 0;
            off_time = low_off_time;
            off = new cMessage("off");
            scheduleAt(simTime() + off_time, off);
            delete msg;
        }else{
            if(sensor_on){
                if(!strcmp(msg->getName(),"LRB")){
                    if(waiting_LRB){
                        // get LRB, wait for SRB
                        if(!discoveried){
                          discovery_time = discovery_time + 1;
                          discoveried = true;
                          // remove extra energy
                          energy_discovery = energy_discovery - p_rx * (Ton - (simTime().dbl() - tmp_time));
                        }
                        // switch to high duty cycle and wait for SR-Beacon
                        waiting_LRB = 0;
                        waiting_SRB = 1;
                        off_time = high_off_time;
                        cancelAndDelete(on);
                        // add SRB timeout
                        LRB_timeout = new cMessage("LRB_timeout");
                        is_LRB_timeout = 1;
                        scheduleAt(simTime() + LRB_timeout_time, LRB_timeout);
                        off = new cMessage("off");
                        scheduleAt(simTime() + off_time, off);
                        delete msg;
                    }else{
                        delete msg;
                    }
                }else if(!strcmp(msg->getName(),"SRB")){
                   if(waiting_SRB){
//                       if(!discoveried){
//                          discovery_time = discovery_time + 1;
//                          discoveried = true;
//                          // remove extra energy
//                          energy_discovery = energy_discovery - p_rx * (Ton - (simTime().dbl() - tmp_time));
//                       }
                       // start transmission
                       if(is_LRB_timeout){
                           cancelAndDelete(LRB_timeout);
                       }
                       is_LRB_timeout = 0; // reset states
                       transmitting = 1;
                       waiting_LRB = 0;
                       waiting_SRB = 0;

                       packet = new cMessage("packet");
                       send(packet,"out");
                       // energy send packet
                       energy_transfer += p_tx * P_duration;

                       bubble("Send packet.");

                       // set packet_timeout timer
                       packet_timeout = new cMessage("packet_timeout");
                       scheduleAt(simTime() + 0.028001, packet_timeout);
                       delete msg;
                   }else{
                       delete msg;
                   }
                }
            }
            else{
                bubble("sensor off, delete");
                delete msg;
            }
        }

    }
}
