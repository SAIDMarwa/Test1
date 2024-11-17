#include "LoRaWANNetwork.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>

#define NUM_GATEWAYS 3  // Nombre de passerelles (clusters) si on veut changer il faut changer la ligne 167
#define MAX_ITERATIONS 100
#define DIM 2  // Dimension (x, y)
#define NUM_NODES 100  // Nombre de nœuds, ajuster selon le fichier

typedef struct {
    double x, y;  // Position du nœud
} Node;

typedef struct {
    double x, y;  // Centre du cluster (passerelle)
} Gateway;



namespace LoRaWANNetwork {
// Fonction pour extraire la valeur numérique d'une chaîne
double extractValue(const char* str) {
    double value;
    sscanf(str, "%lf", &value);
    return value;
}

double distance(Node a, Gateway b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
void assignNodesToGateways(Node nodes[], Gateway gateways[], int assignments[], int numNodes, int numGateways) {
    for (int i = 0; i < numNodes; i++) {
        double minDistance = distance(nodes[i], gateways[0]);
        int clusterIndex = 0;

        // Trouver la passerelle la plus proche
        for (int j = 1; j < numGateways; j++) {
            double dist = distance(nodes[i], gateways[j]);
            if (dist < minDistance) {
                minDistance = dist;
                clusterIndex = j;
            }
        }
        assignments[i] = clusterIndex;
    }
}

void updateGateways(Node nodes[], Gateway gateways[], int assignments[], int numNodes, int numGateways) {
    for (int i = 0; i < numGateways; i++) {
        double sumX = 0, sumY = 0;
        int count = 0;

        // Calculer le centre de chaque cluster
        for (int j = 0; j < numNodes; j++) {
            if (assignments[j] == i) {
                sumX += nodes[j].x;
                sumY += nodes[j].y;
                count++;
            }
        }

        if (count > 0) {
            gateways[i].x = sumX / count;
            gateways[i].y = sumY / count;
        } else {
            // Si aucun nœud n'est assigné à cette passerelle, réinitialiser à une position aléatoire
            gateways[i].x = (rand() % 100);  // Valeur aléatoire, ajuster la plage selon votre environnement
            gateways[i].y = (rand() % 100);
        }
    }
}
void kMeans(Node nodes[], Gateway gateways[], int numNodes, int numGateways) {
    int assignments[numNodes];
    int iterations = 0;
    bool assignmentsChanged = true; // Initialisation à true pour entrer dans la boucle
    // Répéter jusqu'à convergence ou nombre maximum d'itérations
    do {
        assignmentsChanged = false; // Ne pas redéclarer la variable ici

        // Assigner les nœuds aux passerelles les plus proches
        assignNodesToGateways(nodes, gateways, assignments, numNodes, numGateways);

        // Sauvegarder les anciennes positions des passerelles
        Gateway oldGateways[numGateways];
        for (int i = 0; i < numGateways; i++) {
            oldGateways[i] = gateways[i];
        }

        // Mettre à jour les passerelles
        updateGateways(nodes, gateways, assignments, numNodes, numGateways);

        // Vérifier si les passerelles ont changé
        for (int i = 0; i < numGateways; i++) {
            if (gateways[i].x != oldGateways[i].x || gateways[i].y != oldGateways[i].y) {
                assignmentsChanged = true;
            }
        }

        iterations++;
        if (iterations >= MAX_ITERATIONS) break;

    } while (assignmentsChanged);
}

double calculateInertia(Node nodes[], Gateway gateways[], int assignments[], int numNodes, int numGateways) {
    double inertia = 0.0;
    for (int i = 0; i < numNodes; i++) {
        double dist = pow(nodes[i].x - gateways[assignments[i]].x, 2) +
                      pow(nodes[i].y - gateways[assignments[i]].y, 2);
        inertia += dist;
    }
    return inertia;
}


int findOptimalClusters(Node nodes[], int numNodes) {
    const int maxClusters = 10;  // Tester jusqu'à 10 clusters
    double inertias[maxClusters];

    for (int k = 1; k <= maxClusters; k++) {
        Gateway gateways[k];
        int assignments[numNodes];

        // Initialiser des passerelles aléatoires
        for (int i = 0; i < k; i++) {
            gateways[i].x = nodes[i % numNodes].x;
            gateways[i].y = nodes[i % numNodes].y;
        }

        // Appliquer K-Means
        kMeans(nodes, gateways, numNodes, k);

        // Calculer l'inertie
        assignNodesToGateways(nodes, gateways, assignments, numNodes, k);
        inertias[k - 1] = calculateInertia(nodes, gateways, assignments, numNodes, k);
    }

    // Trouver le coude
    int optimalK = 1;
    for (int i = 1; i < maxClusters - 1; i++) {
        double decrease1 = inertias[i - 1] - inertias[i];
        double decrease2 = inertias[i] - inertias[i + 1];

        if (decrease1 > decrease2) {
            optimalK = i + 1;
            break;
        }
    }

    return optimalK;
}

// Fonction pour charger les positions des nœuds depuis un fichier de configuration
int loadNodesFromFile(Node nodes[], const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erreur d'ouverture du fichier %s\n", filename);
        return -1;
    }

    char line[256];
    int i = 0;

    while (fgets(line, sizeof(line), file) != NULL && i < NUM_NODES) {
        if (strstr(line, "initialX") != NULL) {
            // Extraire la position X
            char *start = strchr(line, '=') + 1;
            char *end = strchr(start, 'm');
            *end = '\0';  // Couper à la position du 'm'
            nodes[i].x = extractValue(start);
        }

        if (strstr(line, "initialY") != NULL) {
            // Extraire la position Y
            char *start = strchr(line, '=') + 1;
            char *end = strchr(start, 'm');
            *end = '\0';  // Couper à la position du 'm'
            nodes[i].y = extractValue(start);
            i++;  // Compter le nœud une fois que X et Y sont extraits
        }
    }

    fclose(file);
    return i;  // Retourne le nombre de nœuds lus
}
void saveGatewaysToFile(Gateway gateways[], int numGateways, const char* filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erreur d'ouverture du fichier %s\n", filename);
        return;
    }

    for (int i = 0; i < numGateways; i++) {
           fprintf(file, "**.loRaGW[%d].initialX = %.2f\n", i, gateways[i].x);
           fprintf(file, "**.loRaGW[%d].initialY = %.2f\n", i, gateways[i].y);
       }


    fclose(file);
}

int main() {
    Node nodes[NUM_NODES];
    const char *filename = "node_positions.txt";  // Nom du fichier contenant les positions des nœuds

    // Charger les positions des nœuds depuis le fichier
    int numNodes = loadNodesFromFile(nodes, filename);
    if (numNodes == -1) {
        return 1;  // Erreur de lecture du fichier
    }

    // Déterminer le nombre optimal de clusters
       int optimalClusters = findOptimalClusters(nodes, numNodes);
       EV << "Nombre optimal de clusters trouvé : " << optimalClusters << "\n";

       Gateway gateways[optimalClusters];

       // Initialiser les passerelles
       for (int i = 0; i < optimalClusters; i++) {
           gateways[i].x = nodes[i % numNodes].x;
           gateways[i].y = nodes[i % numNodes].y;
       }

       // Exécuter K-Means avec le nombre optimal de clusters
       kMeans(nodes, gateways, numNodes, optimalClusters);

    // Afficher les positions des passerelles
    printf("Positions des passerelles optimisées :\n");
    for (int i = 0; i < NUM_GATEWAYS; i++) {
        printf("Passerelle %d: (%.2f, %.2f)\n", i+1, gateways[i].x, gateways[i].y);
    }
    saveGatewaysToFile(gateways, NUM_GATEWAYS, "gateway_positions.txt");
    return 0;
}



void LoRaWANNetwork::initialize()
{
    // Initialisez ici les variables ou programmez des événements.
    EV << "Initialization complete for LoRaWAN network module" << endl;

}

} // namespace LoRaWANNetwork
