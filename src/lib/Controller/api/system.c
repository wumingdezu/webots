/*
 * Copyright 1996-2019 Cyberbotics Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>  // strlen
#include <sys/stat.h>

#include <webots/utils/system.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

#ifdef _WIN32
static char *buffer = NULL;

static void free_buffer() {
  free(buffer);
}
#endif

const char *wbu_system_getenv(const char *variable) {
#ifdef _WIN32
  wchar_t *value;
  size_t size = strlen(variable) + 1;
  wchar_t *wvariable = (wchar_t *)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, variable, -1, wvariable, size);
  size = GetEnvironmentVariableW(wvariable, NULL, 0);
  if (size == 0)
    return NULL;  // not defined
  value = (wchar_t *)malloc(size * sizeof(wchar_t));
  GetEnvironmentVariableW(wvariable, value, size);
  free(wvariable);
  size = WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
  if (buffer == NULL)
    atexit(free_buffer);
  else
    free(buffer);
  buffer = (char *)malloc(size);
  WideCharToMultiByte(CP_UTF8, 0, value, -1, buffer, size, NULL, NULL);
  free(value);
  return (const char *)buffer;
#else
  return getenv(variable);
#endif
}

const char *wbu_system_short_path(const char *path) {
#ifdef _WIN32
  int size = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
  wchar_t *w_path = (wchar_t *)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, path, -1, w_path, size);
  size = GetShortPathNameW(w_path, NULL, 0);
  wchar_t *w_short_path = (wchar_t *)malloc(size * sizeof(wchar_t));
  GetShortPathNameW(w_path, w_short_path, size);
  free(w_path);
  size = WideCharToMultiByte(CP_UTF8, 0, w_short_path, -1, NULL, 0, NULL, NULL);
  if (buffer == NULL)
    atexit(free_buffer);
  else
    free(buffer);
  buffer = (char *)malloc(size);
  WideCharToMultiByte(CP_UTF8, 0, w_short_path, -1, buffer, size, NULL, NULL);
  return (const char *)buffer;
#else
  return path;
#endif
}

static const char *wbu_system_tmp_path() {
  static char *tmp = NULL;
  if (tmp)
    return tmp;
  const char *WEBOTS_TMP_PATH = getenv("WEBOTS_TMP_PATH");
  if (WEBOTS_TMP_PATH && WEBOTS_TMP_PATH[0]) {
    tmp = (char *)WEBOTS_TMP_PATH;
    return tmp;
  }
#ifdef _WIN32
  const char *LOCALAPPDATA = getenv("LOCALAPPDATA");
  assert(LOCALAPPDATA && LOCALAPPDATA[0]);
  const size_t len = strlen(LOCALAPPDATA) + 6;  // adding "\\Temp"
  tmp = malloc(len);
  snprintf(tmp, len, "%s\\Temp", LOCALAPPDATA);
#elif defined(__linux__)
  tmp = "/tmp";
#elif defined(__APPLE__)
  tmp = "/var/tmp";
#endif
  return tmp;
}

const char *wbu_system_webots_tmp_path() {
  static const char *WEBOTS_TMP_PATH = NULL;
  if (WEBOTS_TMP_PATH)
    return WEBOTS_TMP_PATH;
  WEBOTS_TMP_PATH = getenv("WEBOTS_TMP_PATH");
  if (WEBOTS_TMP_PATH && WEBOTS_TMP_PATH[0])
    return WEBOTS_TMP_PATH;
  const char *tmp = wbu_system_tmp_path();
  const size_t l = strlen(tmp);
  const char *WEBOTS_PID = getenv("WEBOTS_PID");
  int webots_pid = 0;
  if (WEBOTS_PID && strlen(WEBOTS_PID) > 0)
    sscanf(WEBOTS_PID, "%d", &webots_pid);
  const size_t path_buffer_size = l + 32;  // enough room to hold tmp + "/webots-XXXX...XXX" (pid_t maximum string length)
  char *path_buffer = malloc(path_buffer_size);
  if (webots_pid == 0) {  // get the webots pid from the most recent "webots-XXX" folder
    DIR *dir;
    dir = opendir(tmp);
    if (dir) {
      struct dirent *entry;
      time_t most_recent = 0;
      while ((entry = readdir(dir))) {
        if (strncmp(entry->d_name, "webots-", 7) == 0) {
          struct stat s;
          snprintf(path_buffer, path_buffer_size, "%s/%s", tmp, entry->d_name);
          if (stat(path_buffer, &s) < 0)
            continue;
          if (!S_ISDIR(s.st_mode))
            continue;
          if (s.st_mtime < most_recent)
            continue;
          sscanf(entry->d_name, "webots-%d", &webots_pid);
          most_recent = s.st_mtime;
        }
      }
      closedir(dir);
    }
  }
  if (webots_pid == 0) {
    free(path_buffer);
    path_buffer = NULL;
  } else
    snprintf(path_buffer, path_buffer_size, "%s/webots-%d", tmp, webots_pid);
  WEBOTS_TMP_PATH = path_buffer;
  return WEBOTS_TMP_PATH;
}
