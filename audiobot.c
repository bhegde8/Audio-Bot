#include <string.h>
#include <jansson.h>

#include <ulfius.h>
#include <u_example.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IPADDR "127.0.0.1"
#define PORT 7437

#define MUSICDIR "/home/jmyers/Desktop/songs/"

#define MUSICYTDLROUTE "/local/music/ytdl"
#define MUSICPLAYROUTE "/local/music/play"
#define MUSICSTOPROUTE "/local/music/stop"

#define TTSROUTE "/local/tts"

int callback_ytdl_music (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_play_music (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_stop_music (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_play_tts (const struct _u_request * request, struct _u_response * response, void * user_data);


#define PORT 7437
#define LOCALPREFIX "/local"

/**
 * Main function
 */
int main (int argc, char **argv) {
  
  // jansson integer type can vary
#if JSON_INTEGER_IS_LONG_LONG
  long long nb_audiobot = 0;
#else
  long nb_audiobot = 0;
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
  ulfius_add_endpoint_by_val(&instance, "POST", MUSICPLAYROUTE, NULL, 1, &callback_play_music, &nb_audiobot);
  ulfius_add_endpoint_by_val(&instance, "POST", MUSICSTOPROUTE, NULL, 1, &callback_stop_music, &nb_audiobot);
  ulfius_add_endpoint_by_val(&instance, "POST", MUSICYTDLROUTE, NULL, 1, &callback_ytdl_music, &nb_audiobot);
  
  ulfius_add_endpoint_by_val(&instance, "POST", TTSROUTE, NULL, 1, &callback_play_tts, &nb_audiobot);
  
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
  
  json_t * json_nb_audiobot = ulfius_get_json_body_request(request, NULL);

  char * tts_param_message = json_string_value(json_object_get(json_nb_audiobot, "message"));
  char * tts_param_pitch = json_string_value(json_object_get(json_nb_audiobot, "pitch"));
  char * tts_param_vol = json_string_value(json_object_get(json_nb_audiobot, "volume"));
  char * tts_param_speed = json_string_value(json_object_get(json_nb_audiobot, "speed"));

  char buffer[512];
  if(snprintf(buffer, sizeof(buffer), "espeak-ng -a %s -s %s -p %s \"%s\"", tts_param_vol, tts_param_speed, tts_param_pitch, tts_param_message)>=sizeof(buffer))
  {
    return U_CALLBACK_CONTINUE;
  }
  else
  {
    system(buffer);
  }
  
  return U_CALLBACK_CONTINUE;
}

int callback_play_music (const struct _u_request * request, struct _u_response * response, void * user_data) {

  json_t * json_nb_audiobot = ulfius_get_json_body_request(request, NULL);

  char * music_param_song = json_string_value(json_object_get(json_nb_audiobot, "song"));
  
  char buffer[256];
  if(snprintf(buffer, sizeof(buffer), "cvlc --no-video --play-and-exit \"file://%s%s\"", MUSICDIR, music_param_song)>=sizeof(buffer))
  {
    return U_CALLBACK_CONTINUE;
  }
  else
  {
    system(buffer);
  } 
 
  return U_CALLBACK_CONTINUE;
}

int callback_stop_music (const struct _u_request * request, struct _u_response * response, void * user_data) {
  system("killall vlc");

  return U_CALLBACK_CONTINUE;
}

int callback_ytdl_music (const struct _u_request * request, struct _u_response * response, void * user_data) {

  json_t * json_nb_audiobot = ulfius_get_json_body_request(request, NULL);

  char * music_param_url = json_string_value(json_object_get(json_nb_audiobot, "url"));
  char * music_param_name = json_string_value(json_object_get(json_nb_audiobot, "name"));
  
  char buffer[384];
  if(snprintf(buffer, sizeof(buffer), "youtube-dl -o \"%s%s.flv\" \"%s\"", MUSICDIR, music_param_name, music_param_url)>=sizeof(buffer))
  {
    return U_CALLBACK_CONTINUE;
  }
  else
  {
    system(buffer);
  } 
 
  return U_CALLBACK_CONTINUE;
}
