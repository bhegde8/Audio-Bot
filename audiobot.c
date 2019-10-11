#include <string.h>
#include <jansson.h>

#include <ulfius.h>
#include <u_example.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IPADDR "127.0.0.1"
#define PORT 7437

#define LOCALPREFIX "/local"
#define TTSPREFIX "/local/tts"

int callback_play_testsounds (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_play_tts (const struct _u_request * request, struct _u_response * response, void * user_data);

#define PORT 7437
#define LOCALPREFIX "/local"

int callback_play_testsounds (const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * Main function
 */
int main (int argc, char **argv) {
  
  // jansson integer type can vary
#if JSON_INTEGER_IS_LONG_LONG
  long long nb_sheep = 0;
#else
  long nb_sheep = 0;
#endif
  
  // Initialize the instance
  struct _u_instance instance;

  struct sockaddr_in ipAddress;

  memset(&ipAddress, '\0', sizeof(ipAddress)); 
  ipAddress.sin_family = AF_INET;
  ipAddress.sin_port = htons(PORT);
  ipAddress.sin_addr.s_addr = inet_addr(IPADDR);
  
  
  if (ulfius_init_instance(&instance, PORT, &ipAddress, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  
  // Endpoint list declaration
  // The first 3 are webservices with a specific url
  // The last endpoint will be called for every GET call and will serve the static files
  ulfius_add_endpoint_by_val(&instance, "POST", LOCALPREFIX, NULL, 1, &callback_play_testsounds, &nb_sheep);
  ulfius_add_endpoint_by_val(&instance, "POST", TTSPREFIX, NULL, 1, &callback_play_tts, &nb_sheep);
  
  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start audiobot on port %u\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    printf("Error starting framework\n");
  }
    
  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  
  y_close_logs();
  
  return 0;
}

int callback_play_tts (const struct _u_request * request, struct _u_response * response, void * user_data) {

  system("espeak-ng \"Test\"");

  /* json_body = json_object();
  json_object_set_new(json_body, "nbsheep", json_integer(* nb_sheep));
  ulfius_set_json_body_response(response, 200, json_body);
  json_decref(json_nb_sheep);
  json_decref(json_body); */
  return U_CALLBACK_CONTINUE;
}

int callback_play_testsounds (const struct _u_request * request, struct _u_response * response, void * user_data) {

  system("espeak-ng \"Test\"");
  system("aplay /home/jmyers/Desktop/songs/jojopart3.wav");
  
  /* json_body = json_object();
  json_object_set_new(json_body, "nbsheep", json_integer(* nb_sheep));
  ulfius_set_json_body_response(response, 200, json_body);
  json_decref(json_nb_sheep);
  json_decref(json_body); */
  return U_CALLBACK_CONTINUE;
}

