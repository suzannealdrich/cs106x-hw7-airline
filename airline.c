/* * File:   airline.c * Author: Suzanne Aldrich * ----------------- * This is the Hit or Miss Airline reservation software. */#include <stdio.h>#include <ctype.h>#include "genlib.h"#include "simpio.h"#include "strlib.h"#include "random.h"#include "squeue.h"#include "symtab.h"#include "reserve.h"bool Execute(int cmd, flight *plane);void Reserve(flight *plane);void Cancel(flight *plane);void SeatChart(flight *plane);void PassList(flight *plane);void FindPass(flight *plane);void CheckSeats(flight *plane);void ShowStandbys(flight *plane);void ReadFile(flight *plane);void PrintStandbys(void *value);void PrintSeated(flight *plane, resRec *party);int IntQuestion(string prompt, int min, int max);main(){ 	 flight *plane = InitPlane(); 	 int cmd; 	  	 Randomize(); 	 printf("Welcome to the Hit or Miss Airline reservation system.\n"); 	 do {	 	printf("\n");	 	printf("0:  Quit System\n");	 	printf("1:  Reserve Party\n");	 	printf("2:  Cancel Party\n");	 	printf("3:  Print Seating Chart\n");	 	printf("4:  Print Passenger List\n");	 	printf("5:  Find Passenger Record\n");	 	printf("6:  Check Availability\n");	 	printf("7:  Show Standbys\n");	 	printf("8:  Read Reservation File\n");	 	cmd = IntQuestion("Your Choice?", 0, 8);	 } while (Execute(cmd, plane));	 ExitToShell(); } bool Execute(int cmd, flight *plane){	 	switch (cmd) { 	case 0: 	printf("Goodbye.\n"); 	 return FALSE; 	case 1: 	Reserve(plane); 				break; 	case 2: 	Cancel(plane); 					break; 	case 3: 	SeatChart(plane); 				break; 	case 4: 	PassList(plane); 				break; 	case 5: 	FindPass(plane); 				break; 	case 6: 	CheckSeats(plane); 				break; 	case 7:	 	ShowStandbys(plane); 			break; 	case 8:		ReadFile(plane);				break; 	default: 	printf("Not a valid command.\n"); break; 	} 	return TRUE;}void Reserve(flight *plane){	resRec *party;	int i;		party  = InitParty();	party->partySize = IntQuestion("Number of passengers:", 1, plane->numSeats);	party->nameList = GetBlock(party->partySize * sizeof(string));	for (i = 0; i < party->partySize; i++) {		printf("Passenger name (last, first): ");		party->nameList[i] = ConvertToLowerCase(GetLine());		Enter(plane->nameTable, party->nameList[i], party);	}	party->priority = 		(vipT) IntQuestion("Priority (1 = Standard, 2 = Emergency, 3 = Premier):", 1, 3);	if (SeatParty(party, plane))  {		for (i = 0; i < party->partySize; i++) {			printf("\t%s is in row %c, seat %d\n", 				party->nameList[i], party->row + 'A', party->seat + i + 1);		}	} else {		printf("\tUnable to seat party.\n");		printf("\tThe party has been placed on standby.\n");	}	printf("\tConfirmation Number: %s\n", party->id);	}void Cancel(flight *plane){	string passenger;	resRec *party;	int i;		printf("Passenger to cancel (last, first): ");	passenger = ConvertToLowerCase(GetLine());	party = Lookup(plane->nameTable, passenger);	if (party == UNDEFINED) {		printf("\tPassenger not found\n");		return;	}	switch (party->seatingStatus) {	case seated:  		RemoveSeated(plane, party);		break;	case standby:		RemoveStandby(plane, party); 		break;	default: 		printf("\tParty already cancelled\n");		return; 	}	party->seatingStatus = cancelled;	printf("\tThe following party of %d has been cancelled:\n", 		party->partySize);	for (i = 0; i < party->partySize; i++) {		printf("\t\t%s\n", party->nameList[i]);	}	printf("\tProcessing standbys...\n");	SeatStandbys(plane, party->row, PrintSeated);}void SeatChart(flight *plane){	int row, seat, index = 0;	resRec *party;		printf("\t   ");	for (seat = 0; seat < plane->numSeats; seat++) {		printf("%-18d", seat + 1);	}	printf("\n");	for (row = 0; row < plane->numRows; row++) {		printf("\t%c: ", row + 'A');		for (seat = 0; seat < plane->numSeats; seat++) {			party = plane->planeSeats[index++];					printf("%-18s", (party ? party->nameList[seat - party->seat] : "--------"));		}		printf("\n");	}}void PassList(flight *plane){	int row, seat, index = 0;	resRec *party;		for (row = 0; row < plane->numRows; row++) {		for (seat = 0; seat < plane->numSeats; seat++) {			party = plane->planeSeats[index++];			if (party) {				printf("\t%18s (party of %d) is in row %c, seat %d\n", 					party->nameList[seat - party->seat], party->partySize, row + 'A', seat + 1);			}		}	}}void FindPass(flight *plane){	resRec *party;	string passenger;	int i;		printf("Passenger name (last, first): ");	passenger = ConvertToLowerCase(GetLine());	party = Lookup(plane->nameTable, passenger);	if (party == UNDEFINED) {		printf("\tPassenger \"%s\" not found.\n", passenger);		return;	}	printf("\tThat passenger is in a party of %d:\n", party->partySize);	switch (party->seatingStatus) {	case seated:		for (i = 0; i < party->partySize; i++) {			printf("\t\t%s is in row %c, seat %d\n", 				party->nameList[i], party->row + 'A', party->seat + i + 1);		}		break;	case standby:		printf("\tThat party is on standby with %s priority:\n", PriorityToStr(party->priority));		for (i = 0; i < party->partySize; i++) {			printf("\t\t%s\n", party->nameList[i]);		}		break;	case cancelled:		printf("\tThat party has been cancelled.\n");		break;	}}void CheckSeats(flight *plane){	printf("\tThere are %d seats available.\n", plane->seatsLeft);}void ShowStandbys(flight *plane){	printf("The standbys are:\n");	if (IsEmpty(plane->standbyQueue)) printf("\tNo standby parties.\n");	MapSpecialQueue(plane->standbyQueue, PrintStandbys);	}void PrintStandbys(void *value){	resRec *party = (resRec *) value;	int i;		printf("\tParty of %d with %s priority:\n", party->partySize, PriorityToStr(party->priority));	for (i = 0; i < party->partySize; i++) {		printf("\t\t%s\n", party->nameList[i]);	}}void ReadFile(flight *plane){	FILE *f;	string filename;	resRec *party;	string line;	int i;		printf("Reservation filename (press return for default): ");	filename = GetLine();	if (!filename[0]) filename = DEFAULT_FILE;	f = fopen(filename, "r");	if (!f) {		printf("File is non-existant.\n"); 		return;	}	while ((line = ReadLine(f)) != NULL) {		party = InitParty();		party->partySize = StringToInteger(line);		party->nameList = GetBlock(party->partySize * sizeof(string));		for (i = 0; i < party->partySize; i++) {			party->nameList[i] = ConvertToLowerCase(ReadLine(f));			Enter(plane->nameTable, party->nameList[i], party);		}		party->priority = StrToPriority(ReadLine(f));		SeatParty(party, plane);	}	fclose(f);}int IntQuestion(string prompt, int min, int max){	int variable;	repeat {		printf("%s  ", prompt);		variable = GetInteger();		if (min <= variable && variable <= max) return variable;		printf("\tValue must be between %d and %d.\n", min, max);	} }void PrintSeated(flight *plane, resRec *party){	int i;		printf("\tThe following party of %d has been seated:\n", party->partySize);	for (i = 0; i < party->partySize; i++) {		printf("\t\t%s is in row %c, seat %d\n", 			party->nameList[i], party->row + 'A', party->seat + i + 1);	}}