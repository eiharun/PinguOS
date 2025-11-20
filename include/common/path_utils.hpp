#pragma once
#include <common/types.h>
namespace common {

class PathUtils {
public:
    // Copy string safely with bounds checking
    static void safe_strcpy(char* dest, const char* src, size_t max_len) {
        size_t i = 0;
        while (i < max_len - 1 && src[i] != '\0') {
            dest[i] = src[i];
            i++;
        }
        dest[i] = '\0';
    }
    
    // Compare strings
    static bool strcmp(const char* a, const char* b) {
        size_t i = 0;
        while (a[i] != '\0' && b[i] != '\0') {
            if (a[i] != b[i]) return false;
            i++;
        }
        return a[i] == b[i];
    }
    
    // String length
    static size_t strlen(const char* str) {
        size_t len = 0;
        while (str[len] != '\0') len++;
        return len;
    }
    
    // Split path into components
    // Returns false if path is invalid
    static bool split_path(const char* path, char components[][256], 
                          uint32_t max_components, uint32_t& count) {
        count = 0;
        if (path[0] != '/') return false;  // Must be absolute
        
        size_t i = 1;  // Skip leading '/'
        size_t comp_idx = 0;
        
        while (path[i] != '\0' && count < max_components) {
            if (path[i] == '/') {
                if (comp_idx > 0) {
                    components[count][comp_idx] = '\0';
                    count++;
                    comp_idx = 0;
                }
                i++;
            } else {
                if (comp_idx < 255) {
                    components[count][comp_idx++] = path[i];
                }
                i++;
            }
        }
        
        // Handle last component
        if (comp_idx > 0 && count < max_components) {
            components[count][comp_idx] = '\0';
            count++;
        }
        
        return true;
    }
    
    static int get_index(const char* src, char target){
        int i{};
        int dot_index = -1;
        while (src[i] != '\0') {
            if (src[i] == target) dot_index = i;
            i++;
        }
        return dot_index;
    }

    static bool split_filename_ext(const char* filename, char* components[2]){
        bool split{false};
        int len_name{};
        int len_ext{};
        for(int i=0; filename[i]!='.'; ++i){
            if(filename[i]=='\0'){
                components[0][len_name] = '\0';
                return split;
            }
            components[0][i] = filename[i];
            len_name++;
        }
        components[0][len_name] = '\0';
        split = true;
        for(int i=0; filename[i]!='\0'; ++i){
            components[1][i] = filename[i];
            len_ext++;
        }
        components[1][len_ext] = '\0';
        return split;
    }

    // Compare filename (8.3 format) with pattern
    static bool match_filename(const char* filename, const char* pattern) {
        // Simple exact match for now
        return strcmp(filename, pattern);
    }

    static void merge_path(const char* first, const char* second, char* dest){
        int len{};
        for(int i=0; first[i]!='\0'; ++i){
            dest[len++]=first[i];
        }
        if(dest[len-1] != '/'){
            dest[len++] = '/';
        }
        for(int s=0; second[s]!='\0'; s++){
            dest[len++]=second[s];
        }
        dest[len] = '\0';
    }
};

}
