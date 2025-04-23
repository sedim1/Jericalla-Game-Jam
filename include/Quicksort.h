#ifndef QUICKSORT_H
#define QUICKSORT_H

#include "game.h"

int partition(Sprite arr[], Player*p,int low, int high) {
  
    Sprite pivot = arr[high];
    pivot.dist = distance(p->x,(float)pivot.x,p->y,(float)pivot.y);
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
        arr[j].dist = distance(p->x,(float)arr[j].x,p->y,(float)arr[j].y);
        if (arr[j].dist < pivot.dist) {
            i++;
            Sprite aux = arr[i];
            arr[i] = arr[j];
            arr[j] = aux;
        }
    }
    Sprite aux = arr[i+1]; arr[i+1] = arr[high]; arr[high] = aux;
    return i + 1;
}

void quickSort(Sprite arr[], Player* p,int low, int high) {
    if (low < high) {
        int pi = partition(arr,p , low, high);
        quickSort(arr,p, low, pi - 1);
        quickSort(arr, p, pi + 1, high);
    }
}


#endif
