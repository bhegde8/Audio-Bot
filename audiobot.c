#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include <jansson.h>

#include <ulfius.h>
#include <u_example.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IPADDR "127.0.0.1"
#define PORT 7437

#define STATIC_FOLDER "static"

#define MUSICDIR "/home/jmyers/Desktop/songs/"

#define MUSICLISTROUTE "/local/music/list"
#define MUSICYTDLROUTE "/local/music/ytdl"
#define MUSICPLAYROUTE "/local/music/play"
#define MUSICSTOPROUTE "/local/music/stop"

#define TTSROUTE "/local/tts"

int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_list_music (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_ytdl_music (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_play_music (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_stop_music (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_play_tts (const struct _u_request * request, struct _u_response * response, void * user_data);

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

  struct _u_map mime_types;
  u_map_init(&mime_types);
  u_map_put(&mime_types, ".html", "text/html");
  u_map_put(&mime_types, ".css", "text/css");
  u_map_put(&mime_types, ".js", "application/javascript");
  u_map_put(&mime_types, ".png", "image/png");
  u_map_put(&mime_types, ".jpeg", "image/jpeg");
  u_map_put(&mime_types, ".jpg", "image/jpeg");
  u_map_put(&mime_types, "*", "application/octet-stream");
  
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "POST", MUSICPLAYROUTE, NULL, 1, &callback_play_music, &nb_audiobot);
  ulfius_add_endpoint_by_val(&instance, "POST", MUSICSTOPROUTE, NULL, 1, &callback_stop_music, &nb_audiobot);
  ulfius_add_endpoint_by_val(&instance, "POST", MUSICYTDLROUTE, NULL, 1, &callback_ytdl_music, &nb_audiobot);
  ulfius_add_endpoint_by_val(&instance, "POST", MUSICLISTROUTE, NULL, 1, &callback_list_music, &nb_audiobot);
  
  ulfius_add_endpoint_by_val(&instance, "POST", TTSROUTE, NULL, 1, &callback_play_tts, &nb_audiobot);

  ulfius_add_endpoint_by_val(&instance, "GET", "*", NULL, 1, &callback_static_file, &mime_types);
  
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
  json_t * json_response = NULL;

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
    json_response = json_object();
    json_object_set_new(json_response, "success", json_integer(1));
    ulfius_set_json_body_response(response, 200, json_response);
    json_decref(json_response);

    system(buffer);
  }
  
  return U_CALLBACK_CONTINUE;
}

int callback_play_music (const struct _u_request * request, struct _u_response * response, void * user_data) {

  json_t * json_nb_audiobot = ulfius_get_json_body_request(request, NULL);
  json_t * json_response = NULL;
  
  char * music_param_song = json_string_value(json_object_get(json_nb_audiobot, "name"));
  
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
  json_t * json_response = NULL;

  char * music_param_url = json_string_value(json_object_get(json_nb_audiobot, "url"));
  char * music_param_name = json_string_value(json_object_get(json_nb_audiobot, "name"));
  
  char buffer[384];
  if(snprintf(buffer, sizeof(buffer), "youtube-dl -o \"%s%s.flv\" \"%s\"", MUSICDIR, music_param_name, music_param_url)>=sizeof(buffer))
  {
    return U_CALLBACK_CONTINUE;
  }
  else
  {
    json_response = json_object();
    json_object_set_new(json_response, "success", json_integer(1));
    ulfius_set_json_body_response(response, 200, json_response);
    json_decref(json_response);
    
    system(buffer);
  }

  return U_CALLBACK_CONTINUE;
}

int callback_list_music (const struct _u_request * request, struct _u_response * response, void * user_data) {

  /*
  json_t * json_nb_audiobot = ulfius_get_json_body_request(request, NULL);
  
  char * auth_param_password = json_string_value(json_object_get(json_nb_audiobot, "pwd"));
  */
  json_t * json_response = NULL;
  json_t * music_array = json_array();
  
  DIR * music_dir;
  struct dirent * dir;

  music_dir = opendir(MUSICDIR);

  if(music_dir) {

    while ((dir = readdir(music_dir)) != NULL) {

      char * file_name = dir->d_name;

      if(file_name[0] != '.') {
	json_array_append(music_array, json_string(file_name));
      }
      
    }

    closedir(music_dir);

  }

  json_response = json_object();
  json_object_set_new(json_response, "list", music_array);
  ulfius_set_json_body_response(response, 200, json_response);
  json_decref(json_response);
  
  return U_CALLBACK_CONTINUE;
}

const char * get_filename_ext(const char *path) {
    const char *dot = o_strrchr(path, '.');
    if(!dot || dot == path) return "*";
    return dot + 1;
}

int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  void * buffer = NULL;
  long length;
  FILE * f;
  char  * file_path = msprintf("%s%s", STATIC_FOLDER, request->http_url);
  const char * content_type;
  
  if (access(file_path, F_OK) != -1) {
    f = fopen (file_path, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = o_malloc(length*sizeof(void));
      if (buffer) {
        fread (buffer, 1, length, f);
      }
      fclose (f);
    }

    if (buffer) {
      content_type = u_map_get((struct _u_map *)user_data, get_filename_ext(request->http_url));
      response->binary_body = buffer;
      response->binary_body_length = length;
      u_map_put(response->map_header, "Content-Type", content_type);
      response->status = 200;
    } else {
      response->status = 404;
    }
  } else {
    response->status = 404;
  }
  o_free(file_path);
  return U_CALLBACK_CONTINUE;
}
