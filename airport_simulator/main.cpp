#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include <math.h>
#include <list> 
#include <iterator> 
using namespace std;

#define PER_DIEUM 200

int cost_per_hour = 315;
int charge_per_hour = 700;
int cost_per_plane = 80000;
int fractional_owners = 0; //per plane
int home_count = 0;

#define NUM_CITIES 100
#define NUM_PLANES 15

#define SPEED 180
#define POPULATION 60

						   
#define RADIUS 500
#define HUB_CITY 0

#define PRICE_SENSITIVITY .6
#define MIN_DIST 100

#define PRINT 0

#define pi 3.14159265358979323846

int total = 0;
int cash = 0;
long long revenu = 0;
int total_population = 0;
int deadhead_count = 0;
int flight_count = 0;
int overbooked_count = 0;
float hours_flown = 0;

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
//	int city_id;
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
}FLIGHT;

FLIGHT schedule[365][256];

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
		for (ii = 0; ii < 256; ii++)
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
			
			if ((!strcmp(continent, "NA") && !strcmp(iso_country, "US")) && (get_distance(city.location,home_city.location) < radius))
	//		if (!strcmp(iso_region, "US-WA") || !strcmp(iso_region, "US-OR") || !strcmp(iso_region, "US-CA"))
			//if ((!strcmp(continent, "NA") && (get_distance(city.location, home_city.location) < radius)))
			{
			//	if (city.type == LARGE_AIRPORT )
				if (city.type == LARGE_AIRPORT || city.type == MEDIUM_AIRPORT || city.type == SMALL_AIRPORT)
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
	else if (num < (24 + 21 + 12 + 11 + 5 + 6 + 3 + 3 + 15))
	{
		traveler.travel_chance = 20 / 365.0;
	}

	traveler.travel_chance *= PRICE_SENSITIVITY;

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
	return (rand() % 24);
}

TRIP get_trip(TRAVELER traveler)
{
	TRIP trip;
	float chance;

	chance = (float(rand()) / RAND_MAX);

	if ((1 - chance) < traveler.travel_chance)
	{
		trip.going = true;
		trip.outbound.source_city = traveler.city;
		trip.outbound.destination_city = get_random_city(traveler.city.id);
		trip.outbound.departure_time = get_departure_time();
		trip.outbound.full = true;
		trip.stay_length = get_stay_length();
		trip.inbound.source_city = trip.outbound.destination_city;
		trip.inbound.destination_city = trip.outbound.source_city;
		trip.inbound.departure_time = get_departure_time();
		trip.inbound.full = true;

		trip.outbound.ETE = get_distance(trip.outbound.source_city.location, trip.outbound.destination_city.location) / SPEED;
		trip.inbound.ETE = trip.outbound.ETE;
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
	for (jj = 0; jj < NUM_PLANES; jj++)
	{
		for (ii = 0; ii < 24; ii++)
		{
			fleet[jj].busy[ii] = false;
		}
	}

	return 0;
}

PLANE find_plane(int source_city_id, int hour, float ETE)
{
	int plane_id;
	int ii;
	int jj;
	PLANE deadhead_options[NUM_PLANES];
	PLANE  plane;



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

	for (ii = 0; ii < NUM_PLANES; ii++)
	{
		deadhead_options[ii].distance = get_distance(cities[fleet[ii].city_id].location, cities[source_city_id].location);
		deadhead_options[ii].plane_id = ii;

	}

	qsort(deadhead_options, NUM_PLANES, sizeof(PLANE), compare_distance);

		for (ii = 0; ii < NUM_PLANES; ii++)
		{
			plane.booked = false;

			for (jj = 0; jj < ETE + 1; jj++)
			{
				if (fleet[deadhead_options[ii].plane_id].busy[hour + jj] == true)
			//	if (fleet[ii].busy[hour + jj] == true)
				{
					plane.booked = true;
					break;
				}
			}
			if (plane.booked == false)
			{
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

		for (ii = -1; ii <= ETE + 2; ii++)
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

	int flight_count = 0;

	init_schedule();

	for (ii = 0; ii < NUM_CITIES; ii++)
	{
		for (jj = 0; jj < cities[ii].population; jj++)
		{
			traveler = get_traveler(cities[ii]);

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

					if (ll > 256)
					{
						printf("fix me\n");
						//		ll = 1 / 0;
					}

					schedule[kk][ll] = trip.outbound;
					flight_count++;

					if (kk + trip.stay_length < 365)
					{

						ll = 0;
						while (schedule[kk + trip.stay_length][ll].going)
						{
							ll++;
						}
						schedule[kk + trip.stay_length][ll] = trip.inbound;
						flight_count++;
					}
				}
			}
		}
	}

	for (kk = 0; kk < 365; kk++)
	{
		qsort(schedule[kk], 256, sizeof(FLIGHT), compare_departure_time);
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

	FILE *file;

	errno_t err;
	float ETE;

	err = fopen_s(&file, "C:\\Users\\bhoward\\Documents\\Visual Studio 2015\\Projects\\airport_simulator\\flight_history.csv", "w");

	if (err != 0)
	{
		printf("close the file\n");
		return 0;
	}

	for (kk = 0; kk < 365; kk++)
	{
		ii = 0;
		while (schedule[kk][ii].going)
		{
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
					paid_hours_flown += schedule[kk][ii].ETE;
					fleet[plane.plane_id].city_id = schedule[kk][ii].destination_city.id;

					fprintf(file, "%f,%f,0 %f,%f,0\n", schedule[kk][ii].source_city.location.lon, schedule[kk][ii].source_city.location.lat, schedule[kk][ii].destination_city.location.lon, schedule[kk][ii].destination_city.location.lat);

					if (PRINT)
					{
						printf("%d ", kk);
						print_flight(schedule[kk][ii]);
					}

					flight_count++;
					cash -= cost_per_hour*schedule[kk][ii].ETE;
					cash += charge_per_hour*schedule[kk][ii].ETE;
					revenu += charge_per_hour*schedule[kk][ii].ETE;

					if (PRINT)
					{
						for (ll = 0; ll < NUM_PLANES; ll++)
						{
							printf("%s, ", cities[fleet[ll].city_id].code);
						}
						printf("\n");
					}
				}
				else
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

						//do the flight
						checkout_plane(plane.plane_id, schedule[kk][ii].departure_time, schedule[kk][ii].ETE);
						hours_flown += schedule[kk][ii].ETE;
						paid_hours_flown += schedule[kk][ii].ETE;
						fleet[plane.plane_id].city_id = schedule[kk][ii].destination_city.id;


						fprintf(file, "%f,%f,0 %f,%f,0\n", schedule[kk][ii].source_city.location.lon, schedule[kk][ii].source_city.location.lat, schedule[kk][ii].destination_city.location.lon, schedule[kk][ii].destination_city.location.lat);


						flight_count++;
						cash -= cost_per_hour*schedule[kk][ii].ETE;
						cash += charge_per_hour*schedule[kk][ii].ETE;
						revenu += charge_per_hour*schedule[kk][ii].ETE;
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

		reset_plane_availability();
	}


	total_overbooked_count += overbooked_count;

	cash -= ((overbooked_count*charge_per_hour*.5));
	cash -= cost_per_plane*NUM_PLANES;
	cash -= NUM_PLANES*PER_DIEUM * 365;

	cash += (cost_per_plane / 16)* fractional_owners * NUM_PLANES;

	for (ll = 0; ll < NUM_CITIES; ll++)
	{
		total_population += cities[ll].population;
	}

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

	//38.987095, -94.843528

	
	err = fopen_s(&file, "C:\\Users\\bhoward\\Documents\\Visual Studio 2015\\Projects\\airport_simulator\\profit.csv", "w");

	if (err != 0)
	{
		printf("close the file\n");
		return 0;
	}

	srand((unsigned)time(&t));

	printf("loading cities\n");
	load_cities(NUM_CITIES, RADIUS, khio);

	for (ii = 1; ii < 101; ii++)
	{

		total_population = 0;
		overbooked_count = 0;

		/*
		cash = 0;
		revenu = 0;
		total_population = 0;
		deadhead_count = 0;
		flight_count = 0;
		overbooked_count = 0;
		hours_flown = 0;
		home_count = 0;
		*/
		

		printf("loading planes\n");
		load_planes();

		printf("creating schedule\n");
		create_schedule();

		printf("running schedule\n");
		run_schedule();

		system("cls");

		/*
					printf("customer base %d\n", total_population);
			printf("flights per customer %2.1f\n",(float)(flight_count - deadhead_count)/total_population);
			printf("overbooked count %d\n", overbooked_count);
			printf("overbooked percent %2.2f\n", (float)overbooked_count/ (flight_count - deadhead_count));
			printf("fleet size %d\n",NUM_PLANES);
			printf("total flights %d paid flights %d deadheads %d\npercent deadhead %2.0f\n", flight_count, flight_count-deadhead_count, deadhead_count, (float)deadhead_count*100 / flight_count);
			printf("flights per day per plane %2.1f\n", (float)flight_count / (NUM_PLANES * 365));
			printf("hours flown per plane per year %4.0f average flight length %2.1f\n",hours_flown/NUM_PLANES, hours_flown/flight_count);
	//		printf("avg nights home %d\n", home_count / (1 *NUM_PLANES));
			printf("cash %d\n", cash);
			printf("revenu %9d\n", revenu);
	//		printf("       222111000\n", revenu);

			printf("profit margin %2.1f\n", (float)cash*100 / revenu);
		*/

			printf("customer base %d\n", total_population);
			printf("flights per customer %2.1f\n",(float)(flight_count - deadhead_count)/(total_population*ii));
			printf("overbooked count %d\n", total_overbooked_count /ii);
			printf("overbooked percent %d\n", total_overbooked_count*100/ (flight_count - deadhead_count));
			printf("fleet size %d\n",NUM_PLANES);
			printf("flights per day per plane %2.1f\n", (float)flight_count / (NUM_PLANES * 365 * ii));
			printf("total flights %d paid flights %d deadheads %d\npercent deadhead flights %2.0f\n", flight_count/ii, (flight_count-deadhead_count)/ii, deadhead_count/ii, (float)deadhead_count*100 / flight_count);
			printf("hours flown per plane per year %4.0f average deadhead flight length %2.1f average paid flight length %2.1f\n",hours_flown/(NUM_PLANES*ii), (hours_flown-paid_hours_flown) / deadhead_count,paid_hours_flown / (flight_count - deadhead_count));
			printf("percent deadhead hours %2.2f\n", (float)(hours_flown-paid_hours_flown)/hours_flown);

			printf("avg nights home %d\n", home_count / (NUM_PLANES*ii));
			printf("cash %d\n", cash/(ii));
			printf("revenu %d\n", revenu/ii);
	//		printf("       222111000\n", revenu);

			printf("profit margin %2.1f\n", (float)cash*100 / revenu);
		
	//		total += cash;

		fprintf(file, "%d,\n", cash/ii);
	}

//	printf("Avg yearly profit %d", total/ii);

	fclose(file);

	c = getchar();
}