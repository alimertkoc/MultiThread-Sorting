
/*
    Muhammet Onur Bayraktar 170419821
    Bahadır Şahin           170419822
    Ali Mert Koç            170418018
*/

/*
    POSIX Threads kullanarak N adet
    dosya icerisindeki k. en buyuk sayiyi bulan
    ve buldugu degerleri bir cikti dosyasina yazan
    programdir.     
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

/* 
    Her bir threadin ayri ayri kullanacagi degerleri tutan
    struct yapisi
*/
typedef struct thread_data {
    char filename[20];
    int k;
    int N;
    int topK;
} thread_data;

int findTopK(int *arr, int k, int n)
{
    int tmp = 0;

    for (int i = 0; i < k + 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }

    return arr[n - k - 1];
}

void sort(int *arr, int n)
{
    int tmp;

    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (arr[j] < arr[j + 1])
            {
                tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

int** getNumbersFromFile(char* filename) {
    int fd = open(filename, O_RDONLY);
    int** return_val = malloc(sizeof(int*) * 2);

    // Dizinin INT_MAX adet elemani olmasi icin static olarak olusturulmasi gerekir.
    // Tum threadler ayni processin kaynaklarini kullandigindan
    // degisken static olarak tanimlanamazdi.
    int inputs[100001];
    int input_index = 0, tmp_num_index = 0;
    char tmp_c, tmp_num[11];

    while(read(fd, &tmp_c, 1) != 0) {
        if(tmp_c == ' ') {
            tmp_num[tmp_num_index] = '\0';
            tmp_num_index = 0;
            sscanf(tmp_num, "%d", &inputs[input_index]);
            input_index++;
        }

        else {
            tmp_num[tmp_num_index] = tmp_c;
            tmp_num_index++;
        }
    }

    close(fd);
    return_val[0] = &input_index;
    return_val[1] = inputs;
    return return_val;
}

void* findTopKThread(void* arg) {
    int** output;
    int topK;

    thread_data *t_data = (thread_data*) arg;

    output = getNumbersFromFile(t_data -> filename);
    topK = findTopK(output[1], t_data -> k, *output[0]);
    t_data -> topK = topK;
    pthread_exit(NULL);
}

void printArray(int *arr, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        printf("%d, ", arr[i]);
    }

    printf("%d\n", arr[n - 1]);
}


void main(int argc, char* argv[]) {
    
    /*
        Program execute edilirken verilen
        argumanlar degiskenlere atanir.
    */

    int k, N;

    sscanf(argv[1], "%d", &k);
    sscanf(argv[2], "%d", &N);

    int outputs[N];

    char filenames[N][20], outfile[20];

    for (int i = 3; i < argc - 1; i++)
    {
        strcpy(filenames[i - 3], argv[i]);
    }

    strcpy(outfile, argv[argc - 1]);
    
    pthread_t pth[N]; // Threadleri tutan diziler olusturuldu
    thread_data t_data[N];

    // Threadlerin olusturulmasi
    for(int i = 0; i < N; i++) {
        // k ve N degerlerinin struct degiskeni
        // icerisine atanmasi
        t_data[i].k = k;
        t_data[i].N = N;
        strcpy(t_data[i].filename, filenames[i]);
        pthread_create(&pth[i], NULL, findTopKThread, (void *) &t_data[i]);
    }

    for(int j = 0; j < N; j++) {
        // Main fonksiyonunun beklemesi icin yazilan kod
        pthread_join(pth[j], NULL);
        outputs[j] = t_data[j].topK;
    }

    sort(outputs, N);
    printf("****POSIX Threads****\n"); // make komutunun testi icin
    // printArray(outputs, N);

    /* Ciktilari dosyaya yazma islemi */
    int out_fd = open(outfile, O_CREAT | O_WRONLY, S_IRUSR);
    char result[11];

    for (int k = 0; k < N; k++)
    {
        sprintf(result, "%d", outputs[k]);
        strcat(result, "\n");
        write(out_fd, result, strlen(result));
    }

    close(out_fd);
}