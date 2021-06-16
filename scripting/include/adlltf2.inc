#if defined _adlltf2_included
  #endinput
#endif
#define _adlltf2_included

forward void adlltf2_CheaterDetected      (const int client, const int numofevents);
forward void adlltf2_ProcessedListenEvents(const int client, const int numofevents);

public Extension __ext_adlltf2 =
{
	name = "adlltf2",
	file = "adlltf2.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};
