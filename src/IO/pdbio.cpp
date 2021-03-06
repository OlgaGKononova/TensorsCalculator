//#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pdbio.h"

/*
 * pdbio.c
 *
 *  Created on: Nov 9, 2008
 *      Author: zhmurov
 */

#define buf_size 80
//#define DEBUGPDBIO

/*
 * Private methods
 */
//void parseAtomLine(PDB* pdbData, char* line, int currentAtom);
void parseSSBondLine(PDB* pdbData, char* line, int currentSSBond);


/*
 * Parses data from PDB (Protein Data Bank) file format into PDB object.
 * Only ATOM and SSBOND entries are considered, some fields are ignored
 * (marked befor corresponding method).
 *
 * For detailed information about PDB file format, please visit
 * http://www.wwpdb.org/documentation/format23/sect6.html
 *
 * Parameters:
 * 		filename: name of a pdb file to parse
 * 		pdbData: pointer of an object to save data into
 */

void readPDB(const char* filename, PDB* pdbData){
	printf("Reading %s.\n", filename);
	int ss_count = 0, atoms_count = 0;
	char buffer[buf_size];
	FILE* file = fopen(filename, "r");
	if ( file != NULL ){
		while(fgets(buffer, buf_size, file) != NULL && strncmp(buffer, "END", 3) != 0){
			if(strncmp(buffer,"SSBOND",6) == 0){
				ss_count++;
			}
			if(strncmp(buffer, "ATOM", 4) == 0 || strncmp(buffer, "HETATM", 6) == 0){
				atoms_count++;
			}
		}
		printf("Found %d atoms.\n", atoms_count);

		pdbData->atomCount = atoms_count;
		pdbData->ssCount = ss_count;
		pdbData->atoms = (Atom*)malloc(atoms_count*sizeof(Atom));
		pdbData->ssbonds = (SSBond*)malloc(ss_count*sizeof(SSBond));

		int current_atom = 0;
		int current_ss = 0;

		rewind(file);

		while(fgets(buffer, buf_size, file) != NULL && strncmp(buffer, "END", 3) != 0){
			char* pch = strtok(buffer, " ");
			if(strcmp(pch, "SSBOND") == 0){
				parseSSBondLine(pdbData, buffer, current_ss);
				current_ss++;
			}
			if(strcmp(pch, "ATOM") == 0 || strcmp(pch, "HETATM") == 0){
				parseAtomLine(pdbData, buffer, current_atom);
				current_atom ++;
			}

		}
	printf("Done reading '%s'.\n", filename);
	fclose(file);
	} else {
		perror(filename);
		exit(0);
	}
}

/*
 * Parses single line of 'ATOM' entry from pdb file.
 * ATOM entry format in PDB:
 *
 * COLUMNS      DATA TYPE        FIELD      DEFINITION
 * ------------------------------------------------------
 *  1 -  6      Record name      "ATOM    "
 *  7 - 11      Integer          id		    Atom serial number.
 * 13 - 16      Atom             name       Atom name.
 * 17           Character        altLoc     Alternate location indicator.
 * 18 - 20      Residue name     resName    Residue name.
 * 22           Character        chainID    Chain identifier.
 * 23 - 26      Integer          resSeq     Residue sequence number.
 * 27           AChar            iCode      Code for insertion of residues. (Ignored)
 * 31 - 38      Real(8.3)        x          Orthogonal coordinates for X in Angstroms
 * 39 - 46      Real(8.3)        y          Orthogonal coordinates for Y in Angstroms
 * 47 - 54      Real(8.3)        z          Orthogonal coordinates for Z in Angstroms
 * 55 - 60      Real(6.2)        occupancy  Occupancy.
 * 61 - 66      Real(6.2)        tempFactor Temperature factor.
 * 77 - 78      LString(2)       element    Element symbol, right-justified. (Ignored)
 * 79 - 80      LString(2)       charge     Charge on the atom. (Ignored)
 *
 * For more details visit http://www.wwpdb.org/documentation/format23/sect9.html
 *
 *
 * Method parameters:
 * 		pdbData:	pointer to pdbData object to read into
 * 		line: line from the file to parse
 * 		currentSSBond: indicates location in array of ssbonds
 * 			where new bond should be saved
 */

void parseAtomLine(PDB* pdbData, char* line, int currentAtom){
	char id[6];
	char atomName[5];
	char resName[4], chain, altLoc;
	char resid[5];
	char x[9], y[9], z[9];
	char occupancy[7];
	char beta[7];
	char segment[5];
	strncpy(id, &line[6], 5);
	id[5] = '\0';
	strncpy(atomName, &line[12], 4);
	atomName[4] = '\0';
	altLoc = line[16];
	strncpy(resName, &line[17], 3);
	resName[3] = '\0';
	strncpy(&chain, &line[21], 1);
	strncpy(resid, &line[22], 4);
	resid[4] = '\0';
	strncpy(x, &line[30], 8);
	x[8] = '\0';
	strncpy(y, &line[38], 8);
	y[8] = '\0';
	strncpy(z, &line[46], 8);
	z[8] = '\0';
	strncpy(occupancy, &line[54], 6);
	occupancy[6] = '\0';
	strncpy(beta, &line[60], 6);
	beta[6] = '\0';
	strncpy(segment, &line[72], 4);
	segment[4] = '\0';
	int i;
	for(i = 0; i < 5; i++){
		if(segment[i] == '\n' || segment [i] == '\r'){
			segment[i] = ' ';
		}
	}
	strcpy(pdbData->atoms[currentAtom].name, strtok(atomName, " "));
	pdbData->atoms[currentAtom].altLoc = altLoc;
	pdbData->atoms[currentAtom].name[4] = 0;
	pdbData->atoms[currentAtom].chain = chain;
	pdbData->atoms[currentAtom].resid = atoi(resid);
	strcpy(pdbData->atoms[currentAtom].resName, resName);
	pdbData->atoms[currentAtom].resName[3] = 0;
	pdbData->atoms[currentAtom].id = atoi(id);
	pdbData->atoms[currentAtom].x = atof(x);
	pdbData->atoms[currentAtom].y = atof(y);
	pdbData->atoms[currentAtom].z = atof(z);
	pdbData->atoms[currentAtom].occupancy = atof(occupancy);
	pdbData->atoms[currentAtom].beta = atof(beta);
	pdbData->atoms[currentAtom].charge = 0.0;
	strcpy(pdbData->atoms[currentAtom].segment, segment);
#ifdef DEBUGPDBIO
	printAtom(pdbData->atoms[currentAtom]);
#endif
}

/*
 * Parses single line of 'SSBOND' entry from pdb file.
 * SSBOND entry format in PDB:
 *
 * COLUMNS        DATA TYPE       FIELD         DEFINITION
 * -------------------------------------------------------------------
 *  1 -  6        Record name     "SSBOND"
 *  8 - 10        Integer         serNum       Serial number.
 * 12 - 14        LString(3)      "CYS"        Residue name.
 * 16             Character       chainID1     Chain identifier.
 * 18 - 21        Integer         seqNum1      Residue sequence number.
 * 22             AChar           icode1       Insertion code. (Ignored)
 * 26 - 28        LString(3)      "CYS"        Residue name.
 * 30             Character       chainID2     Chain identifier.
 * 32 - 35        Integer         seqNum2      Residue sequence number.
 * 36             AChar           icode2       Insertion code. (Ignored)
 * 60 - 65        SymOP           sym1         Symmetry oper for 1st resid (Ignored)
 * 67 - 72        SymOP           sym2         Symmetry oper for 2nd resid (Ignored)
 *
 * For more details visit http://www.wwpdb.org/documentation/format23/sect6.html
 *
 * Method parameters:
 * 		pdbData:	pointer to pdbData object to read into
 * 		line: line from the file to parse
 * 		currentSSBond: indicates location in array of ssbonds
 * 			where new bond should be saved
 */
void parseSSBondLine(PDB* pdbData, char* line, int currentSSBond){
	char chain;
	char resid[4];
	strncpy(&chain, &line[15], 1);
	strncpy(resid, &line[17], 4);
	pdbData->ssbonds[currentSSBond].chain1 = chain;
	pdbData->ssbonds[currentSSBond].resid1 = atoi(resid);
	strncpy(&chain, &line[29], 1);
	strncpy(resid, &line[31], 4);
	pdbData->ssbonds[currentSSBond].chain2 = chain;
	pdbData->ssbonds[currentSSBond].resid2 = atoi(resid);
#ifdef DEBUG
	printf("SS #%d: %c%d - %c%d\n", current_ss, pdbData->ssbonds[current_ss].chain1, pdbData->ssbonds[current_ss].resid1,
			pdbData->ssbonds[current_ss].chain2, pdbData->ssbonds[current_ss].resid2);
#endif
}

/*
 * Saves data from PDB object into a PDB file format
 *
 * For detailed information about PDB file format, please visit
 * http://www.wwpdb.org/documentation/format23/sect6.html
 *
 * Parameters:
 * 		filename: name of a file to save into (will be overwritten/created)
 * 		pdbData: pointer of an object to get data from
 */
void writePDB(const char* filename, PDB* pdbData){
	FILE* file = fopen(filename, "w");
	int i;
	for(i = 0; i < pdbData->ssCount; i++){
		fprintf(file, "SSBOND %3d CYS %c %4d    CYS %c %4d\n",
								i + 1,
								pdbData->ssbonds[i].chain1,
								pdbData->ssbonds[i].resid1,
								pdbData->ssbonds[i].chain2,
								pdbData->ssbonds[i].resid2);
	}
	for(i = 0; i < pdbData->atomCount; i++){
		fprintf(file, "ATOM  %5d %-4s%c%3s %c%4d    %8.3f%8.3f%8.3f%6.2f%6.2f      %s       \n",
								i + 1,
								pdbData->atoms[i].name,
								pdbData->atoms[i].altLoc,
								pdbData->atoms[i].resName,
								pdbData->atoms[i].chain,
								pdbData->atoms[i].resid,
								pdbData->atoms[i].x,
								pdbData->atoms[i].y,
								pdbData->atoms[i].z,
								pdbData->atoms[i].occupancy,
								pdbData->atoms[i].beta,
								pdbData->atoms[i].segment);
	}
	fprintf(file, "END");
	fclose(file);
}

void appendPDB(const char* filename, PDB* pdbData, int connect){
	//printf("Append frame to pdb...\n");
	FILE* file = fopen(filename, "a");
	int i;
	for(i = 0; i < pdbData->ssCount; i++){
		fprintf(file, "SSBOND %3d CYS %c %4d    CYS %c %4d\n",
								i + 1,
								pdbData->ssbonds[i].chain1,
								pdbData->ssbonds[i].resid1,
								pdbData->ssbonds[i].chain2,
								pdbData->ssbonds[i].resid2);
	}
	for(i = 0; i < pdbData->atomCount; i++){
		fprintf(file, "ATOM  %5d %-4s%c%3s %c%4d    %8.3f%8.3f%8.3f%6.2f%6.2f%6.2f      %s       \n",
								i + 1,
								pdbData->atoms[i].name,
								pdbData->atoms[i].altLoc,
								pdbData->atoms[i].resName,
								pdbData->atoms[i].chain,
								pdbData->atoms[i].resid,
								pdbData->atoms[i].x,
								pdbData->atoms[i].y,
								pdbData->atoms[i].z,
								pdbData->atoms[i].occupancy,
								pdbData->atoms[i].beta,
								pdbData->atoms[i].charge,
								pdbData->atoms[i].segment);
	}
	if (connect == 1){
		int lastAmino = -1;
		for(i = 0; i < sop.bondCount; i++){
			if(lastAmino != sop.bonds[i].i){
				if(lastAmino != -1){
					fprintf(file, "\n");
				}
				fprintf(file, "CONECT%5d", sop.bonds[i].i + 1);
				lastAmino = sop.bonds[i].i;
			}
			fprintf(file, "%5d", sop.bonds[i].j + 1);
		}
		fprintf(file, "\n");
	}
	fprintf(file, "END\n");
	fclose(file);
}

/*
 * Prints PDBAtom object in a PDB format.
 * Parameters:
 * 		atomData: PDBAtom object to print
 */
void printAtom(Atom atomData){
	printf("ATOM  %5d %-4s%c%3s %c%4d    %8.3f%8.3f%8.3f%6.2f%6.2f      PROT\n",
			atomData.id,
			atomData.name,
			atomData.altLoc,
			atomData.resName,
			atomData.chain,
			atomData.resid,
			atomData.x,
			atomData.y,
			atomData.z,
			atomData.occupancy,
			atomData.beta);
}

/*
 * Prints PDBAtom object in a PDB format into a file.
 * Parameters:
 * 		atomData: PDBAtom object to print
 */
void printAtomToFile(FILE* file, Atom atomData){
	fprintf(file, "ATOM  %5d %-4s%c%3s %c%4d    %8.3f%8.3f%8.3f%6.2f%6.2f      %c\n",
			atomData.id,
			atomData.name,
			atomData.altLoc,
			atomData.resName,
			' ',
			atomData.resid,
			atomData.x,
			atomData.y,
			atomData.z,
			atomData.occupancy,
			atomData.beta,
			atomData.chain);
}
