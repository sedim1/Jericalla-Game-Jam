#ifndef QUICKSORT_H
#define QUICKSORT_H

#include "game.h"

int partition(Sprite arr[], Player*p,int low, int high) {
  
    // Choose the pivot
    Sprite pivot = arr[high];
    pivot.dist = distance(p->x,(float)pivot.x,p->y,(float)pivot.y);
    // Index of smaller element and indicates 
    // the right position of pivot found so far
    int i = low - 1;

    // Traverse arr[low..high] and move all smaller
    // elements on left side. Elements from low to 
    // i are smaller after every iteration
    for (int j = low; j <= high - 1; j++) {
        arr[j].dist = distance(p->x,(float)arr[j].x,p->y,(float)arr[j].y);
        //printf("%f\n",arr[j].dist);
        if (arr[j].dist < pivot.dist) {
            i++;
            Sprite aux = arr[i];
            arr[i] = arr[j];
            arr[j] = aux;
        }
    }
    
    // Move pivot after smaller elements and
    // return its position
    Sprite aux = arr[i+1]; arr[i+1] = arr[high]; arr[high] = aux;
    return i + 1;
}

// The QuickSort function implementation
void quickSort(Sprite arr[], Player* p,int low, int high) {
  
    if (low < high) {
        // pi is the partition return index of pivot
        int pi = partition(arr,p , low, high);

        // Recursion calls for smaller elements
        // and greater or equals elements
        quickSort(arr,p, low, pi - 1);
        quickSort(arr, p, pi + 1, high);
    }
}


#endif
