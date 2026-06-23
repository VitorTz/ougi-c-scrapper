#include <stdio.h>
#include "../include/cjson/cJSON.h"
#include "../include/structure/path.h"


// Helper function to process individual items within the main array
void process_json_item(cJSON* item) {
    cJSON* child = NULL;

    // Iterate through all key-value pairs in the JSON object
    cJSON_ArrayForEach(child, item) {
        printf("Key: '%s'\n", child->string);

        // Handle Strings
        if (cJSON_IsString(child)) {
            printf("  -> Type: String | Value: \"%s\"\n", child->valuestring);
        } 
        // Handle Numbers (cJSON stores both int and double)
        else if (cJSON_IsNumber(child)) {
            // A simple check to differentiate float vs int conceptually
            // cJSON populates both valueint and valuedouble.
            if ((double)child->valueint == child->valuedouble) {
                printf("  -> Type: Integer | Value: %d\n", child->valueint);
            } else {
                printf("  -> Type: Float | Value: %f\n", child->valuedouble);
            }
        } 
        // Handle Arrays (Lists)
        else if (cJSON_IsArray(child)) {
            int array_size = cJSON_GetArraySize(child);
            printf("  -> Type: Array | Size: %d\n", array_size);

            if (array_size > 0) {
                cJSON* first_element = cJSON_GetArrayItem(child, 0);

                // Check the type of the first element to determine the list type
                if (cJSON_IsString(first_element)) {
                    printf("    -> Array of Strings: [");
                    cJSON* element = NULL;
                    cJSON_ArrayForEach(element, child) {
                        printf("\"%s\" ", element->valuestring);
                    }
                    printf("]\n");
                } 
                else if (cJSON_IsNumber(first_element)) {
                    // Again, check if the first element acts as a float or int
                    if ((double)first_element->valueint == first_element->valuedouble) {
                        printf("    -> Array of Integers: [");
                        cJSON* element = NULL;
                        cJSON_ArrayForEach(element, child) {
                            printf("%d ", element->valueint);
                        }
                    } else {
                        printf("    -> Array of Floats: [");
                        cJSON* element = NULL;
                        cJSON_ArrayForEach(element, child) {
                            printf("%.2f ", element->valuedouble);
                        }
                    }
                    printf("]\n");
                }
            }
        }
    }
    printf("----------------------------------------\n");
}

int main() {
    // Dummy JSON payload matching your structure requirements
    path_t path = path_create("res/json-test/manhwas.json");
    string_t payload = path_read_text(&path);

    // 1. Parse the raw JSON string into a cJSON tree
    cJSON* root = cJSON_Parse(string_cstr(&payload));
    if (root == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return 1;
    }

    // 2. Ensure the root is an array
    if (cJSON_IsArray(root)) {
        cJSON* item = NULL;
        
        // 3. Iterate through each object in the root array
        cJSON_ArrayForEach(item, root) {
            if (cJSON_IsObject(item)) {
                process_json_item(item);
            }
        }
    } else {
        printf("Root is not a JSON array.\n");
    }

    // 4. Clean up memory
    // This recursively frees all objects, strings, and arrays allocated by cJSON_Parse
    cJSON_Delete(root);
    path_destroy(&path);
    string_free(&payload);

    return 0;
}