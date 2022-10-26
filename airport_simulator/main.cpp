#define _CRT_SECURE_NO_WARNINGS

//new

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include <math.h>
#include <list> 
#include <iterator> 
using namespace std;

#define PER_DIEM 0

int per_diem_count = 0;

int cost_per_hour = 1600; //345
int charge_per_hour = 1900;
int cost_per_plane = 0; //135000
int fractional_owners = 0; //per plane in 1/16ths
int home_count = 0;
int loan_ammount = 0;
float interest_rate = 0.06;
float management_fee = 0.06;
int flying_days = 0;
int max_customer_flights = 0;
int max_daily_flights = 0;
int idle_count = 0;
int no_flight_customer = 0;
double deadhead_fill_chance = 0;

#define MAX_DAILY_FLIGHTS 512

#define NUM_CITIES 500
#define NUM_PLANES 50

#define SPEED 300
#define POPULATION 50
						   
#define RADIUS 1000
#define MAX_RANGE 1200
#define HUB_CITY 0

//#define PRICE_SENSITIVITY 1.1
#define PRICE_SENSITIVITY .45

#define MIN_DIST 100

#define PRINT 0

//#define CHARGE_DEADHEAD -1
#define CHARGE_DEADHEAD 10

#define DO_DEADHEADS 1
#define NETWORK 1

#define DOWN_TIME 180

#define NO_OVERNIGHT 1

#define pi 3.14159265358979323846

int total = 0;
int cash = 0;
long long revenue = 0;
int total_population = 0;
int deadhead_count = 0;
int flight_count = 0;
int owner_flight_count = 0;
int overbooked_count = 0;
float hours_flown = 0;
float owner_hours_flown;
int total_per_diem_count = 0;
int total_overbooked_count = 0;
float paid_hours_flown = 0;

char pre_string[] = "<Placemark> <name>Untitled Path< / name> <styleUrl>#m_ylw - pushpin< / styleUrl> <LineString> <tessellate>1< / tessellate> <coordinates>";
char post_string[] = "< / coordinates> < / LineString> < / Placemark>";


typedef struct
{
	int speed;
	int range;
	int plane_id;
	int distance;
	int city_id;
	bool busy[24];
	bool booked;
}PLANE;

PLANE fleet[NUM_PLANES];

typedef struct
{
	float lat;
	float lon;
}GPS;

typedef struct
{
	int hour[24];
}DAY;

typedef enum
{
	CLOSED,
	HELIPORT,
	SMALL_AIRPORT,
	MEDIUM_AIRPORT,
	LARGE_AIRPORT,
	UNKNOWN
}AIRPORT_TYPE;

typedef enum
{
	US,
	CN
}ISO_COUNTRY;

typedef enum
{
	US_OR,
	US_WA,
	US_CA
}ISO_REGION;

//ident, type, name, elevation_ft, continent, iso_country, iso_region, municipality, gps_code, iata_code, local_code, coordinates
typedef struct
{
	GPS location;
	AIRPORT_TYPE type;
	ISO_COUNTRY country;
	ISO_REGION region;
	int population;
	char code[5];
	int id;
	int plane_count; //this is super bad, I have a memory error somewhere
}CITY;

//list <CITY> cities;

CITY cities[NUM_CITIES];

typedef struct
{
	float travel_chance;
	int stay_length;
	bool ownership;
	CITY city;

}TRAVELER;


typedef struct
{
	CITY source_city;
	CITY destination_city;
	int departure_time;
	float ETE;
	bool going;
	bool full;
	bool is_owner;
}FLIGHT;

FLIGHT schedule[365][MAX_DAILY_FLIGHTS];

typedef struct
{
	bool going;
	FLIGHT outbound;
	int stay_length;
	FLIGHT inbound;
}TRIP;

double deg2rad(double deg)
{
	return (deg * pi / 180);
}

double rad2deg(double rad)
{
	return (rad * 180 / pi);
}



double get_distance(GPS point1, GPS point2)
{
	double theta, dist;

	if ((point1.lat == point2.lat) && (point1.lon == point2.lon)) 
	{
		return 0;
	}
	else
	{
		theta = point1.lon - point2.lon;
		dist = sin(deg2rad(point1.lat)) * sin(deg2rad(point2.lat)) + cos(deg2rad(point1.lat)) * cos(deg2rad(point2.lat)) * cos(deg2rad(theta));
		dist = acos(dist);
		dist = rad2deg(dist);
		dist = dist * 60 * 1.1515;
//		switch (unit) 
//		{
//		case 'M':
//			break;
//		case 'K':
//			dist = dist * 1.609344;
//			break;
//		case 'N':
//			dist = dist * 0.8684;
//			break;
//		}
		return (dist);
	}
}

int init_schedule()
{
	int ii;
	int kk;

	for (kk = 0; kk < 365; kk++)
	{
		for (ii = 0; ii < MAX_DAILY_FLIGHTS; ii++)
		{
			schedule[kk][ii].going = false;
			schedule[kk][ii].departure_time = 100;
		}
	}
	return 0;
}

int compare_departure_time(const void * a, const void * b)
{

	FLIGHT *flightA = (FLIGHT *)a;
	FLIGHT *flightB = (FLIGHT *)b;

	return (flightA->departure_time - flightB->departure_time);
}


int compare_distance(const void * a, const void * b)
{

	PLANE *planeA = (PLANE *)a;
	PLANE *planeB = (PLANE *)b;

	return (planeA->distance - planeB->distance);
}

//ident,type,name,elevation_ft,continent,iso_country,iso_region,municipality,gps_code,iata_code,local_code,coordinates
int load_cities(int num_cities, int radius, CITY home_city)
{
	int city_id = 0;
	FILE *file;
	errno_t err;
	int ii = 0;
//	int plane_id;
	CITY city;

//	char *strings;

	char line[256];
	char ident[256];
	char type[256];
//	char name[256];
//	char elevation_ft[256];
	char continent[256];
	char iso_country[256];
	char iso_region[256];
//	char municipality[256];
//	char gps_code[256];
//	char iata_code[256];
//	char local_code[256];
//	char coordinates[256];
	char lat[256];
	char lng[256];

	err = fopen_s(&file, "C:\\Users\\bhoward\\Documents\\Visual Studio 2015\\Projects\\airport_simulator\\airports.prn", "r");
	//	err = fopen_s(&file, "C:\\Users\\bhoward\\Documents\\Visual Studio 2015\\Projects\\airport_simulator\\airports_fixed.prn", "r");

	if (err != 0)
	{
		printf("something wrong with file\n");
		return 0;
	}

	fgets(line, 256, file);

	while (ii < num_cities)
	{
		if (fgets(line, 256, file) != NULL)
		{
			//		ident	type	continent	iso_co	untryiso_region	coordinates
			sscanf(line, "%s %s %s %s %s %s %s", ident, type, continent, iso_country, iso_region, lng, lat);

			strcpy(city.code, ident);

			if (type[0] == 'c')
			{
				city.type = CLOSED;
			}
			if (type[0] == 'h')
			{
				city.type = HELIPORT;
			}
			else if (type[0] == 's' && type[1] == 'm')
			{
				city.type = SMALL_AIRPORT;
				city.population = POPULATION *.01;
			}
			else if (type[0] == 'm')
			{
				city.type = MEDIUM_AIRPORT;
				city.population = POPULATION *.1;
			}
			else if (type[0] == 'l')
			{
				city.type = LARGE_AIRPORT;
				city.population = POPULATION;
			}
			else
			{
				city.type = UNKNOWN;
			}

			city.location.lat = atof(lat);
			city.location.lon = atof(lng);

			if (HUB_CITY)
			{
				if (ii == 0)
				{
					city.population = POPULATION*NUM_CITIES;
				}
				else
				{
					city.population = 0;
				}
			}
			else
			{
		//		city.population = POPULATION;
			}
			
				if ( !strcmp(iso_country, "US") && (get_distance(city.location,home_city.location) < radius))
			//if ((type[0] == 'l') && !strcmp(iso_country, "US") && (get_distance(city.location, home_city.location) < radius))
			//if ((!strcmp(continent, "EU")) && (get_distance(city.location, home_city.location) < radius))
			//if (!strcmp(iso_country, "JP") && (get_distance(city.location,home_city.location) < radius))
			//if (!strcmp(iso_country, "GB") && (get_distance(city.location, home_city.location) < radius))

			{
				if (city.type == LARGE_AIRPORT || city.type == MEDIUM_AIRPORT )
			//	if (city.type == LARGE_AIRPORT || city.type == MEDIUM_AIRPORT || city.type == SMALL_AIRPORT)
				{
					if (PRINT)
					{
						printf(line);
					}

					//		cities.push_back(city);

					city.id = ii;

					cities[ii] = city;

					ii++;
				}
			}
		}
		else
		{
			if (ii < NUM_CITIES)
			{
				printf("Ran Out of Cities\n");
				printf("Found %d\n",ii);
			}

			for (ii = ii-1; ii >= 0; ii--)
			{
				printf("%f,%f,0 \n", cities[ii].location.lon, cities[ii].location.lat);
			}

			break;
		}
	}

	fclose(file);

	if (ii == 0)
	{
		printf("No Cities Loaded\n");
	}

	if (PRINT)
	{
	
			for (ii = ii - 1; ii >= 0; ii--)
			{
				printf("%f,%f,0 \n", cities[ii].location.lon, cities[ii].location.lat);
			}
	
	}

	return 0;
}


TRAVELER get_traveler(CITY city)
{
	TRAVELER traveler;
	int num;

	traveler.city = city;
	traveler.ownership = false;

	num = (rand() % 100);

	if (num < 24)
	{
		traveler.travel_chance = 1 / 365.0;
	}
	else if (num < (24 + 21))
	{
		traveler.travel_chance = 2 / 365.0;
	}
	else if (num < (24 + 21 + 12))
	{
		traveler.travel_chance = 3 / 365.0;
	}
	else if (num < (24 + 21 + 12 + 11))
	{
		traveler.travel_chance = 4 / 365.0;
	}
	else if (num < (24 + 21 + 12 + 11 + 5))
	{
		traveler.travel_chance = 5 / 365.0;
	}
	else if (num < (24 + 21 + 12 + 11 + 5 + 6))
	{
		traveler.travel_chance = 6 / 365.0;
	}
	else if (num < (24 + 21 + 12 + 11 + 5 + 6 + 3))
	{
		traveler.travel_chance = 7 / 365.0;
	}
	else if (num < (24 + 21 + 12 + 11 + 5 + 6 + 3 + 3))
	{
		traveler.travel_chance = 8 / 365.0;
	}
	else if (num < (24 + 21 + 12 + 11 + 5 + 6 + 3 + 3 + 10))
	{
		traveler.travel_chance = 20 / 365.0;
	}
	else if (num < (24 + 21 + 12 + 11 + 5 + 6 + 3 + 3 + 10 + 5))
	{
		traveler.travel_chance = 50 / 365.0;
	}



	traveler.travel_chance *= PRICE_SENSITIVITY;

	if (num > (24 + 21 + 12))
	{

		num = (rand() % 100);
		if (num < (fractional_owners * NUM_PLANES * 100) / (total_population*.43))
		{
			traveler.ownership = true;
		}
	}

	return traveler;
}

int get_stay_length()
{
	int stay_length = 0;

		/*
		Nights Personal
		Domestic
		Personal
		International
		Business
		Domestic
		Business
		International
		0-3 41 38 68 57
		4-7 45 29 23 29
		8-14 11 23 6 9
		15-30 3 8 3 3
		31+ 1 2 1 1

		*/

	return rand() % 10;
}

CITY get_random_city(int source_city_id)
{
	int city_id;

	city_id = (rand() % (NUM_CITIES));

	while ((city_id == source_city_id) || (get_distance(cities[source_city_id].location, cities[city_id].location) < MIN_DIST) || (get_distance(cities[source_city_id].location, cities[city_id].location) > RADIUS))
	{
		city_id = (rand() % (NUM_CITIES));
	}

	return cities[city_id];
}

int get_departure_time()
{
	double number, u1,u2;
	int hour;

	u1 = (double)rand() / (double)RAND_MAX;
	u2 = (double)rand() / (double)RAND_MAX;

	number = sqrt(-2.0 * log(u1)) * cos(pi * 2 * u2);

//	printf("%f\n",number);

	number += 3;

	if (number < 0)
	{
		number = 0;
	}
	else if (number > 6)
	{
		number = 6;
	}

	hour = number * 4;

//	printf("%d\n",hour);

	return hour;

	return ((rand() % 22)+1);
}

TRIP get_trip(TRAVELER traveler)
{
	TRIP trip;
	float chance;

	chance = (float(rand()) / RAND_MAX);

	if ((1 - chance) < traveler.travel_chance)
	{
		trip.going = true;
		trip.outbound.is_owner = traveler.ownership;
		trip.outbound.source_city = traveler.city;

		//limit range to something reasonable 
		trip.outbound.destination_city = get_random_city(traveler.city.id);
		while(get_distance(trip.outbound.source_city.location, trip.outbound.destination_city.location) > MAX_RANGE)
		{
			trip.outbound.destination_city = get_random_city(traveler.city.id);
		}

		trip.outbound.departure_time = get_departure_time();
		trip.outbound.full = true;
		trip.stay_length = get_stay_length();
		trip.inbound.is_owner = traveler.ownership;
		trip.inbound.source_city = trip.outbound.destination_city;
		trip.inbound.destination_city = trip.outbound.source_city;
		trip.inbound.departure_time = get_departure_time();
		trip.inbound.full = true;

		trip.outbound.ETE = get_distance(trip.outbound.source_city.location, trip.outbound.destination_city.location) / SPEED;
		trip.inbound.ETE = trip.outbound.ETE;


		//doesnt let it do over midnight trips
		while (trip.outbound.ETE + trip.outbound.departure_time > 24)
		{
			trip.outbound.departure_time = get_departure_time();
		}
	
		while (trip.inbound.ETE + trip.inbound.departure_time > 24)
		{
			trip.inbound.departure_time = get_departure_time();
		}

		if (PRINT)
		{
	//		printf("%d\n", trip.outbound.departure_time);
		}

		//if (trip.outbound.ETE > 5.4)
		//{
		//	printf("bad");
		//}
//		trip.inbound.ETE = get_distance(trip.outbound.source_city.location, trip.outbound.destination_city.location) / SPEED;
		return trip;
	}
	else

	{
		trip.going = false;
		return trip;
	}
}

int print_flight(FLIGHT flight)
{
	if (flight.full == true)
	{
		printf("%s, to, %s\n", cities[flight.source_city.id].code, cities[flight.destination_city.id].code);
		//	printf("flight, %s, to, %s\n", cities[flight.destination_id].code, cities[flight.source_id].code);
	}
	else
	{
		printf("%s, to, %s deadhead\n", cities[flight.source_city.id].code, cities[flight.destination_city.id].code);
	}
	return 0;
}

int reset_plane_availability()
{
	int ii;
	int jj;
	bool used;
	bool down;
	int pilot_count = 0;

	for (jj = 0; jj < NUM_PLANES; jj++)
	{
		used = false;
		if ((rand() % 365) < DOWN_TIME)
		{
			down = true;
		}
		else
		{
			down = false;
		}

		for (ii = 0; ii < 24; ii++)
		{
			if (fleet[jj].busy[ii] == true)
			{
				used = true;
			}

			if (down)
			{
				fleet[jj].busy[ii] = true;
			}
			else
			{
				fleet[jj].busy[ii] = false;
			}
		}
		if (used == true)
		{
			pilot_count++;
		}
	}

	idle_count += NUM_PLANES - pilot_count;

	return pilot_count;
}

float get_deadhead_fill_chance(int dest_id, int source_id)
{
	int kk;
	int ii;
	int count = 0;
	int total = 0;

	for (kk = 0; kk < 365; kk++)
	{

		ii = 0;
		while (schedule[kk][ii].going)
		{
			if ((schedule[kk][ii].destination_city.id == dest_id) && (schedule[kk][ii].source_city.id == source_id))
			{
				count++;
			}
			total++;
			ii++;
		}
	}

	return (float)count / 365;
	//return (float)count / (float)total;
}

PLANE find_plane(int source_city_id, int hour, float ETE)
{
	int plane_id;
	int ii;
	int jj;
	PLANE deadhead_options[NUM_PLANES];
	PLANE  plane;
	int dead_head_time;
	float chance;



	if (hour < 0 || hour > 23)
	{

		plane.booked = true;

		return plane;
	}


	for (ii = 0; ii < NUM_PLANES; ii++)
	{
		plane.booked = false;

		if (fleet[ii].city_id == source_city_id)
		{
			for (jj = 0; jj < ETE+1; jj++)
			{
				if (fleet[ii].busy[hour + jj] == true)
				{
					plane.booked = true;
				}
			}
			if (plane.booked == false)
			{
				return fleet[ii];
			}
		}
	}

	//do deadhead



	for (ii = 0; ii < NUM_PLANES; ii++)
	{
		deadhead_options[ii].distance = get_distance(cities[fleet[ii].city_id].location, cities[source_city_id].location);
		deadhead_options[ii].plane_id = ii;

	}

	if (NETWORK)
	{
		qsort(deadhead_options, NUM_PLANES, sizeof(PLANE), compare_distance);
	}

		for (ii = 0; ii < NUM_PLANES; ii++)
		{
			plane.booked = false;

			for (jj = 0; jj < ETE + 1; jj++)
			{
				if (hour + jj <= 24)
				{
					//this finds a plane that is not busy while I need it
					if (fleet[deadhead_options[ii].plane_id].busy[hour + jj] == true)
						//	if (fleet[ii].busy[hour + jj] == true)
					{
						plane.booked = true;
						break;
					}
				}
				else
				{
					plane.booked = true;
					break;
				}
			}

			dead_head_time = deadhead_options[ii].distance / SPEED;

			if (deadhead_options[ii].distance > MAX_RANGE)
			{
				plane.booked = true;
				dead_head_time = -1;
			}

			for (jj = 0; jj < dead_head_time + 1; jj++)
			{
				//need to make this find a plane that I can get here in time

				if (hour - jj >= 0)
				{

					if (fleet[deadhead_options[ii].plane_id].busy[hour - jj] == true)
						//	if (fleet[ii].busy[hour + jj] == true)
					{
						plane.booked = true;
						break;
					}
				}
				else
				{
					plane.booked = true;
					break;
				}
			}
			if (plane.booked == false)
			{
				chance = get_deadhead_fill_chance(source_city_id, fleet[deadhead_options[ii].plane_id].city_id);

				deadhead_fill_chance += chance;

	//			printf("%f\n",chance);

				return fleet[deadhead_options[ii].plane_id];
			//	return fleet[ii];
			}

		}
	
	plane.booked = true;

	return plane;
}

int checkout_plane(int plane_id, int hour, float ETE)
{
	int ii;
	/*
	if (hour >= 3 && hour <= 20)
	{
		for (ii = -3; ii <= 3; ii++)
		{
			fleet[plane_id].busy[hour+ii] = true;
		}
	}
	else
	{
		fleet[plane_id].busy[hour] = true;
	}
	*/

	//going to run way more effieencet like a real airline 
		for (ii = -1; ii <= ETE; ii++)
		{
			fleet[plane_id].busy[abs(hour + ii)%24] = true;
		}

	return 0;
}

int create_schedule()
{
	int ii;
	int jj;
	int kk;
	int ll;
	TRAVELER traveler;
	TRIP trip;

	int traveler_flight_count = 0;

	no_flight_customer = 0;

	init_schedule();

	for (ii = 0; ii < NUM_CITIES; ii++)
	{
		if (PRINT)
		{
			printf("city %d\n", ii);
		}

		for (jj = 0; jj < cities[ii].population; jj++)
		{

			traveler = get_traveler(cities[ii]);

			traveler_flight_count = 0;

			for (kk = 0; kk < 365; kk++)
			{

				trip = get_trip(traveler);

				if (trip.going)
				{
					

					ll = 0;
					while (schedule[kk][ll].going)
					{
						ll++;
					}


					if (ll > MAX_DAILY_FLIGHTS)
					{
						printf("fix me\n");
						//		ll = 1 / 0;
					}

					schedule[kk][ll] = trip.outbound;
					traveler_flight_count++;


					if (kk + trip.stay_length < 365)
					{

						ll = 0;
						while (schedule[kk + trip.stay_length][ll].going)
						{
							ll++;
						}
						schedule[kk + trip.stay_length][ll] = trip.inbound;
				
						traveler_flight_count++;

			
					}

					/*
					if (DE_NETWORK)
					{


						if (trip.stay_length > 1)
						{

							trip.outbound.departure_time = (trip.outbound.departure_time + 6) % 24;
							trip.inbound.departure_time = abs(trip.inbound.departure_time - 6) % 24;

							ll = 0;
							while (schedule[kk][ll].going)
							{
								ll++;
							}
							schedule[kk][ll] = trip.inbound; // repositioning

							ll = 0;
							while (schedule[kk + trip.stay_length][ll].going)
							{
								ll++;
							}
							schedule[kk + trip.stay_length][ll] = trip.outbound; //repositioning 
						}
					}
					*/

				}
			}

	//		printf("%d\n",traveler_flight_count);

			if (traveler_flight_count > max_customer_flights)
			{
				max_customer_flights = traveler_flight_count;
			}

			if (traveler_flight_count == 0)
			{
				no_flight_customer++;
			}
		}
	}

	for (kk = 0; kk < 365; kk++)
	{
		qsort(schedule[kk], MAX_DAILY_FLIGHTS, sizeof(FLIGHT), compare_departure_time);


	}

	if (PRINT)
	{

		printf("daily flights\n");
		for (kk = 0; kk < 365; kk++)
		{
			ii = 0;
			while (schedule[kk][ii].going)
			{
				ii++;
			}
			printf("%d\n", ii);
		}
	}

	return 0;
}

int run_schedule()
{
	FLIGHT deadhead;
	PLANE plane;
	int ii = 0;
	int kk;
	int ll;
	bool flying;

	FILE *file;

	errno_t err;
	float ETE;

	flying_days = 0;
	per_diem_count = 0;

	err = fopen_s(&file, "C:\\Users\\bhoward\\Documents\\Visual Studio 2015\\Projects\\airport_simulator\\flight_history.csv", "w");

	if (err != 0)
	{
		printf("close the file\n");
		return 0;
	}

	for (kk = 0; kk < 365; kk++)
	{
		flying = false;
		ii = 0;
		while (schedule[kk][ii].going)
		{
			flying = true;

			//if ( schedule[kk][ii].destination_city.id == 0 )

			if ( schedule[kk][ii].destination_city.id == 0 || (get_distance(cities[0].location, schedule[kk][ii].destination_city.location) < (SPEED /2)))
			{
				home_count++;
			}

		//	schedule[kk][ii].
			plane = find_plane(schedule[kk][ii].source_city.id, schedule[kk][ii].departure_time, schedule[kk][ii].ETE);
			if (plane.booked == false)
			{
				if (plane.city_id == schedule[kk][ii].source_city.id)
				{

					checkout_plane(plane.plane_id, schedule[kk][ii].departure_time, schedule[kk][ii].ETE);
					hours_flown += schedule[kk][ii].ETE;
					
					fleet[plane.plane_id].city_id = schedule[kk][ii].destination_city.id;

					fprintf(file, "%f,%f,0 %f,%f,0\n", schedule[kk][ii].source_city.location.lon, schedule[kk][ii].source_city.location.lat, schedule[kk][ii].destination_city.location.lon, schedule[kk][ii].destination_city.location.lat);

					if (PRINT)
					{
						printf("%d ", kk);
						print_flight(schedule[kk][ii]);
					}

					flight_count++;

					if (schedule[kk][ii].is_owner)
					{
						owner_hours_flown += schedule[kk][ii].ETE;
						revenue += cost_per_hour*schedule[kk][ii].ETE;
						owner_flight_count++;

					}
					else
					{
						paid_hours_flown += schedule[kk][ii].ETE;
						cash -= cost_per_hour*schedule[kk][ii].ETE;
						cash += charge_per_hour*schedule[kk][ii].ETE;
						revenue += charge_per_hour*schedule[kk][ii].ETE;
					}

					if (PRINT)
					{
						for (ll = 0; ll < NUM_PLANES; ll++)
						{
							printf("%s, ", cities[fleet[ll].city_id].code);
						}
						printf("\n");
					}
				}
				else if(DO_DEADHEADS)
				{
					//do deadhead
					plane = find_plane(schedule[kk][ii].source_city.id, abs(schedule[kk][ii].departure_time - 6) % 24, 5);
					if (plane.booked == false)
					{
						ETE = get_distance(cities[plane.city_id].location, schedule[kk][ii].source_city.location) / SPEED;
						checkout_plane(plane.plane_id, (int)(abs(schedule[kk][ii].departure_time - ETE - 2)) % 24, ETE);
						hours_flown += ETE;
						fleet[plane.plane_id].city_id = schedule[kk][ii].source_city.id;


						fprintf(file, "%f,%f,0 %f,%f,0\n", cities[plane.city_id].location.lon, cities[plane.city_id].location.lat, schedule[kk][ii].source_city.location.lon, schedule[kk][ii].source_city.location.lat);


						deadhead_count++;
						flight_count++;
						cash -= cost_per_hour*ETE;
						if (CHARGE_DEADHEAD)
						{
							cash += charge_per_hour*ETE;
							revenue += charge_per_hour*ETE;

							if ((rand() % 10) > CHARGE_DEADHEAD-1)
							{
								paid_hours_flown += ETE;
							}
						}

						//do the flight
						checkout_plane(plane.plane_id, schedule[kk][ii].departure_time, schedule[kk][ii].ETE);
						hours_flown += schedule[kk][ii].ETE;
						
						fleet[plane.plane_id].city_id = schedule[kk][ii].destination_city.id;


						fprintf(file, "%f,%f,0 %f,%f,0\n", schedule[kk][ii].source_city.location.lon, schedule[kk][ii].source_city.location.lat, schedule[kk][ii].destination_city.location.lon, schedule[kk][ii].destination_city.location.lat);


						flight_count++;

						if (schedule[kk][ii].is_owner)
						{
							owner_hours_flown += schedule[kk][ii].ETE;
							revenue += cost_per_hour*schedule[kk][ii].ETE;
							owner_flight_count++;
						}
						else
						{
							paid_hours_flown += schedule[kk][ii].ETE;
							cash -= cost_per_hour*schedule[kk][ii].ETE;
							cash += charge_per_hour*schedule[kk][ii].ETE;
							revenue += charge_per_hour*schedule[kk][ii].ETE;
						}
					}
					else
					{
			
						overbooked_count++;
		
					}
				}
			}
			else
			{
				overbooked_count++;
			}

			ii++;
		}

		if (ii - 1 > max_daily_flights)
		{
			max_daily_flights = ii - 1;
		}

/*		if (ii - 1 <= NUM_PLANES)
		{
			per_diem_count += (ii - 1);
		}
		else
		{
			per_diem_count += NUM_PLANES;
		}

		*/

		if (flying == true)
		{
			flying_days++;
		}

		per_diem_count += reset_plane_availability();

	}

	total_per_diem_count += per_diem_count;

	total_overbooked_count += overbooked_count;

	cash -= ((overbooked_count*charge_per_hour*1));
	cash -= cost_per_plane*NUM_PLANES;
	cash -= PER_DIEM * per_diem_count;

	cash += (cost_per_plane / 16.0)* fractional_owners * (1+management_fee) * NUM_PLANES;
	cash += (fractional_owners / 16.0)* loan_ammount * interest_rate * NUM_PLANES;

	revenue += (cost_per_plane / 16.0)* fractional_owners * (1 + management_fee) * NUM_PLANES;
	revenue += (fractional_owners / 16.0)* loan_ammount * interest_rate * NUM_PLANES;

//	for (ll = 0; ll < NUM_CITIES; ll++)
//	{
//		total_population += cities[ll].population;
//	}

	fclose(file);

	return 0;
}


int load_planes()
{
	int ii = 0;
	PLANE plane;

//	plane.speed = SPEED;
	plane.booked = false;

	for (ii = 0; ii < 24; ii++)
	{
		plane.busy[ii] = false;
	}

	if (NUM_CITIES > NUM_PLANES)
	{

		for(ii = 0; ii < NUM_PLANES; ii++)
		{
			plane.city_id = ii;
			plane.plane_id = ii;

			fleet[ii] = plane;
		}
	}
	else
	{
		for (ii = 0; ii < NUM_PLANES; ii++)
		{
			plane.city_id = ii % NUM_CITIES;
			plane.plane_id = ii;

			fleet[ii] = plane;
		}
	}

	return 0;
}

int main()
{
	time_t t;
	int c;
	int ii;
	FILE *file;
	errno_t err;

	CITY khio;

	strcpy(khio.code, "KHIO");

	khio.location.lat = 45.548066;
	khio.location.lon = -122.949218;

	//khio.location.lat = 38.987095;
	//khio.location.lon = -94.843528;

//	khio.location.lat = 40.611013;
//	khio.location.lon = -122.369124;
	

	//khio.location.lat = 35.709989;
	//khio.location.lon = 139.729874;

	//khio.location.lat = 46.8;
	//khio.location.lon = 7.7;

	//35.709989, 139.729874

	//38.987095, -94.843528

	//khio.location.lat = 53.452755;
	//khio.location.lon = -2.307568;

	//khio.location.lat = 40.16043;
	//khio.location.lon = -100.740379;


	
	err = fopen_s(&file, "C:\\Users\\bhoward\\Documents\\Visual Studio 2015\\Projects\\airport_simulator\\profit.csv", "w");

	if (err != 0)
	{
		printf("close the file\n");
		return 0;
	}

	srand((unsigned)time(&t));

	printf("loading cities\n");
	load_cities(NUM_CITIES, RADIUS, khio);

	for (ii = 0; ii < NUM_CITIES; ii++)
	{
		total_population += cities[ii].population;
	}


	for (ii = 1; ii < 101; ii++)
	{


		overbooked_count = 0;

		printf("loading planes\n");
		load_planes();

		printf("creating schedule\n");
		create_schedule();

		printf("running schedule\n");
		run_schedule();

		system("cls");


			printf("customer base %d, owner shares %d\n", total_population, NUM_PLANES*fractional_owners);
			printf("average flights per customer %2.1f\n",(float)(flight_count - deadhead_count)/(total_population*ii));
	
			printf("max flights per customer %d\n",max_customer_flights);
			printf("no flight customers %d\n",no_flight_customer);
			printf("overbooked count %d\n", total_overbooked_count /ii);
			printf("overbooked percent %d\n", total_overbooked_count*100/ (flight_count - deadhead_count));
			printf("deadhead fill chance %f\n", deadhead_fill_chance/deadhead_count);
			printf("fleet size %d\n",NUM_PLANES);
			printf("flying days %d\n",flying_days);
			printf("Idle planes per day %2.1f\n", (idle_count/365)/(float)ii);
			printf("max daily flights %d\n", max_daily_flights);
			printf("flights per day per plane %2.1f\n", (float)flight_count / (NUM_PLANES * 365 * ii));
			printf("total flights %6.0d, paid flights %d, deadhead flights %6.0d, owner flights %6.0f, percent deadhead flights %2.2f\n", flight_count/ii, (flight_count-deadhead_count-owner_flight_count)/ii, deadhead_count/ii, (float)owner_flight_count / ii,(float)deadhead_count / flight_count);
			printf("total hours %6.0f, paid hours %6.0f, deadhead hours %6.0f, owner hours %6.0f, percent deadhead hours %2.2f\n", hours_flown/ii, paid_hours_flown/ii, (hours_flown-paid_hours_flown-owner_hours_flown)/ii, owner_hours_flown/ii, (float)(hours_flown - paid_hours_flown - owner_hours_flown) / hours_flown);
			printf("hours flown per plane per year %4.0f, average deadhead flight length %2.2f, average paid flight length %2.2f\n",hours_flown/(NUM_PLANES*ii), (hours_flown-paid_hours_flown-owner_hours_flown) / deadhead_count,paid_hours_flown / (flight_count - deadhead_count- owner_flight_count));
			

			printf("avg nights home %d\n", home_count / (NUM_PLANES*ii));
			printf("cash    %9.0d\n", cash/(ii));
			printf("        mmmttthhh\n", cash / (ii));
			printf("revenue %9.0d\n", revenue/ii);
	//		printf("       222111000\n", revenu);

			printf("profit margin %2.1f\n", (float)cash*100 / revenue);

			printf("Charge per hour Face Rate %d, Effective Rate %6.0f\n",charge_per_hour, (float)(revenue + (PER_DIEM * total_per_diem_count) - ((cost_per_plane / 16.0)* fractional_owners * (1 + management_fee) * NUM_PLANES)) / paid_hours_flown);

		fprintf(file, "%d,\n", cash/ii);
	}


	fclose(file);

	c = getchar();
}

//	revenue += (cost_per_plane / 16.0)* fractional_owners * (1 + management_fee) * NUM_PLANES;
//revenue += (fractional_owners / 16.0)* loan_ammount * interest_rate * NUM_PLANES;