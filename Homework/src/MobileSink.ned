//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 


//
// TODO auto-generated type
//
simple MobileSinkNode2BD
{
    parameters:
        double v;
        double Tbi;
        double x_start;
        double y_start;
        double x_end;
        double y_end;
        int counter;

    gates:
        input in;
        output out;
}

simple WirelessChannel
{
    parameters:
		int R;
		int r;
		bool packet_loss;
    gates:
        input in[2];
        output out[2];
}

simple SensorNode2BD
{
    parameters:
        double Ton;
		double LDC;
		double HDC;
		double p_rx;
		double p_tx;
		double P_duration;
    gates:
        input in;
        output out;
}

//                @display(500-x,420+y); // center of map
//                500 150
//                @display("i=device/antennatower");
network MobileSink
{
    parameters:
        @display("bgb=1000,700");
    submodules:
        Sink: MobileSinkNode2BD {
            parameters:
                @display("p=500,300;i=device/cellphone"); // center of map
        }
        Wireless: WirelessChannel {
            @display("p=500,400;i=device/antennatower"); // center of map
        }
        Sensor: SensorNode2BD {
            parameters:
                @display("p=500,450;i=device/device"); // ring topology, top-left coordinate
        }
    connections:
		// Sensor <--> Wireless
        Sensor.out --> {  delay = 4ms; } --> Wireless.in[0];
        Wireless.out[0] --> {  delay = 10ms; } --> Sensor.in;

		// Sink <--> Wireless
        Sink.out --> {  delay = 4ms; } --> Wireless.in[1];
        Wireless.out[1] --> {  delay = 10ms; } --> Sink.in;

}
