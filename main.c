#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

char solution_[6];
int is_valid = 0;

char *to_upper(char *s) {
    char *res = malloc(6);
    for (int i = 0; i < 6; ++i) {
        char c = s[i];
        if ('a' <= c && c <= 'z') {
            res[i] = c - ('a' - 'A');
            continue;
        }
        res[i] = c;
    }
    return res;
}

int evaluate_guess(char *guess, char *sol) {
    char colors[5] = {0, 0, 0, 0, 0};
    char guess_copy[6];
    memcpy(guess_copy, guess, 6);
    char sol_copy[6];
    memcpy(sol_copy, sol, 6);
    for (int i = 0; i < 5; ++i) {
        char c = guess[i];
        if (!(('A' <= c && c <= 'Z')||('a' <= c && c <= 'z'))) return -1;
        if (c == sol[i]) {
            colors[i] = 'g';
            guess_copy[i] = 0;
            sol_copy[i] = 0;
        }
    }
    
    for (int i = 0; i < 5; ++i) {
        if (!sol_copy[i]) continue;
        for (int j = 0; j < 5; ++j) {
            if (!guess_copy[j]) continue;
            if (guess_copy[i] == sol_copy[j]) {
                colors[i] = 'y';
                guess_copy[i] = 0;
                sol_copy[j] = 0;
                break;
            }
        }
    }
    
    for (int i = 0; i < 5; ++i) {
        char c = guess[i];
        switch (colors[i]) {
            case 'g':
                printf("\033[0;42m%c\033[0m", c); // green
                break;
            case 'y':
                printf("\033[0;43m%c\033[0m", c); // green
                break;
            default:
                printf("%c", c);
        }
    }

    return 0;
}

size_t set_random_word(void *ptr, size_t size, size_t nmemb, void *userdata) {
    char data[10];
    sprintf(data, "%.*s", (int)(size * nmemb), (char *)ptr);
    sscanf(data, "[\"%5s\"]", solution_);
    return size * nmemb;
}

int get_random_word(void) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://random-word-api.herokuapp.com/word?length=5");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, set_random_word);
        
        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
    return 0;
}

size_t validate_word(void *ptr, size_t size, size_t nmemb, void *userdata) {
    char data[4096];
    snprintf(data, 4096, "%.*s", (int)(size * nmemb), (char *)ptr);
    // printf("data:\n%s\n", data);
    is_valid = 0;
    if (strncmp(data, "[{\"word\":", strlen("[{\"word\":")) == 0) {
        printf("valid\n");
        is_valid = 1;
    } else {
        printf("invalid\n");
    }
    // printf("\n%c\n", data[3]);
    return size * nmemb;
}

int is_guess_word(char *guess) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        // https://api.dictionaryapi.dev/api/v2/entries/en/aaaaa
        char url[sizeof("https://api.dictionaryapi.dev/api/v2/entries/en/aaaaa")];
        sprintf(url, "https://api.dictionaryapi.dev/api/v2/entries/en/%s", guess);
        printf("%s\n", url);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, validate_word);
        
        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
    return 0;
}

int main(void) {
    get_random_word();
    char *wordle = solution_;
    char *solution = to_upper(wordle);
    char guess[7];
    for (int i = 0; i < 7; ++i) {
        printf("guess %d: ", i);
        if (!fgets(guess, 7, stdin)) {
            printf("\nexiting...\n");
            return 0;
        }
        if (guess[0] == '\n') {
            i--;
            continue;
        }
        guess[5] = 0;
        char *guess_upper = to_upper(guess);
        is_guess_word(guess_upper);
        if (!is_valid) {
            printf("not in word list\n");
            i--;
            continue;
        }
        if (evaluate_guess(guess_upper, solution) != 0) {
            printf("uh oh...\n");
            return 1;
        }
        if (strcmp(guess_upper, solution) == 0) {
            printf("\nyou solved it!\n");
            return 0;
        }
        printf("\n");
    }
    printf("you lost!\n");
    printf("The word was '%s'\n", solution);
    return 0;
}
