#include <WString.h>;

/*
Field No.	Example	Format	Name	Unit	Description
0	$PUBX	string	$PUBX	-	Message ID, UBX protocol header, proprietary sentence
1	00	numeric	ID	-	Propietary message identifier: 00
2	081350.00	hhmmss.ss	hhmmss.ss	-	UTC Time, Current time
3	4717.113210	ddmm.mmmm	Latitude	-	Latitude, Degrees + minutes, see Format description
4	N	character	N	-	N/S Indicator, N=north or S=south
5	00833.915187	dddmm.mmmm	Longitude	-	Longitude, Degrees + minutes, see Format description
6	E	character	E	-	E/W indicator, E=east or W=west
7	546.589	numeric	AltRef	m	Altitude above user datum ellipsoid.
8	G3	string	NavStat	-	Navigation Status - Example: G3 =Stand alone 3D solution, NF =No Fix
9	2.1	numeric	Hacc	m	Horizontal accuracy estimate.
10	2.0	numeric	Vacc	m	Vertical accuracy estimate.
11	0.007	numeric	SOG	km/h	Speed over ground
12	77.52	numeric	COG	degrees	Course over ground
13	0.007	numeric	Vvel	m/s	Vertical velocity, positive=downwards
14	-	numeric	ageC	s	Age of most recent DGPS corrections, empty = none available
15	0.92	numeric	HDOP	-	HDOP, Horizontal Dilution of Precision
16	1.19	numeric	VDOP	-	VDOP, Vertical Dilution of Precision
17	0.77	numeric	TDOP	-	TDOP, Time Dilution of Precision
18	9	numeric	GU	-	Number of GPS satellites used in the navigation solution
19	0	numeric	RU	-	Number of GLONASS satellites used in the navigation solution
20	0	numeric	DR	-	DR used
21	*5B	hexadecimal	cs	-	Checksum
22	-	character	<CR><LF>	-	Carriage Return and Line Feed

Navigation Status Description
NF No Fix
DR Dead reckoning only solution
G2 Stand alone 2D solution
G3 Stand alone 3D solution
D2 Differential 2D solution
D3 Differential 3D solution
RK Combined GPS + dead reckoning solution
TT Time only solution
*/

typedef struct {
  String MsgID;
  String sTime;
  String sDate;  // the standard poll message does not return Date, but we will read it anyway using a different function
  String Lat;
  String NS;
  String Long;
  String EW;
  String Alt;
  String Nav;
  String HAcc;
//  String VAcc  
  String Speed;
//  String COG
//  String VVel
//  String AgeC
  String HDOP;
//  String VDOP
//  String TDOP
  String GpsSat;
  String Glonass;
} uBlox_t;

