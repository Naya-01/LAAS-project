#ifndef _VIREMENTS_H_
#define _VIREMENTS_H_

/* struct virement used between server and client */
typedef struct
{
  int numBeneficiaire;
  int numEmetteur;
  int montant;
} structVirement;

#endif