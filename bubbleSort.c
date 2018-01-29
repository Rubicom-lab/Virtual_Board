#include<stdio.h>

//#define ARRAYSIZE 100
int arr[6] = {1,5,4,3};

void bubbleSort(int list[], int n){
	int i,j,temp = 0;
	for(i = n-1; i>0; i--){
	 for(j=0; j<i; j++){

		if(list[j] > list[j+1]){
		 temp = list[j];
		 list[j] = list[j+1];
		 list[j+1] = temp;
		}	
	 }

	}
}


int main(void){
	int len = 4;
	int i=0;
	bubbleSort(arr, len);

}
