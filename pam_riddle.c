#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/random.h>

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT

#include <security/pam_modules.h>
#include <security/pam_ext.h>


#define MAX_INPUT 256
#define MAX_ATTEMPTS 3

struct Riddle_t {
	char * Riddle;
	char * Answer;
};


static char *config_file = "/etc/riddle.conf";


// Returns Riddle_t object, Object is empty if unsuccessful
struct Riddle_t *get_riddle()
{
	FILE *config_file_ptr;
	int line_count = 0;
	struct Riddle_t *riddle;
	char chr;
	char *nl_pos;
	char *line = NULL;
	size_t len = 0;
	uint64_t random = 0;

	// Allocate memory for riddle
	riddle = malloc(sizeof(struct Riddle_t));
	if ( riddle == NULL )
	{
		printf("Unable to allocate memory for Riddle\n");
		return 0;
	}

	// Open config file
	config_file_ptr = fopen(config_file,"r");
	if (config_file_ptr <= 0)
	{
		printf("Unable to open /etc/riddle.conf\n");
		return 0;
	}

	// Get lines
	chr = getc(config_file_ptr);
	while (chr != EOF)
	{
		if (chr == '\n')
		{
			line_count++;
		}
		chr = getc(config_file_ptr);
	}

	// Generate random 64 bit value
	getrandom(&random,4, GRND_NONBLOCK);
	random = random % (line_count - 1); // -1 so doesn't seek end of file

	// Get random line
	rewind(config_file_ptr);
	line_count = 0;
	while(line_count <= random)
	{
		getline(&line,&len,config_file_ptr);
		line_count++;
	}

	// Split line into Riddle + Answer
	char * token = strtok(line, "|");
	riddle->Riddle = token;
	token = strtok(NULL, "|");

	// Remove newline from token
	nl_pos = strchr(token,'\n');
	*nl_pos = 0;
	printf("%s\n",nl_pos);
	riddle->Answer = token;

	// Return riddle object
	return riddle;
}

#ifndef TEST
// Actual Code that checks the authentication
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	int found = 0;
	char *buffer = 0;
	struct Riddle_t *riddle;
	pam_info(pamh,"Please answer the following riddle.");
	riddle = get_riddle();
	if (riddle == 0) {
		pam_info(pamh,"ERROR GETTING RIDDLE");
		return PAM_AUTH_ERR;
	}
	
	pam_info(pamh, riddle->Riddle);
	pam_prompt(pamh, PAM_PROMPT_ECHO_ON, &buffer, "");
	found = strcmp(buffer,riddle->Answer);
	if(found == 0) {
		pam_info(pamh,"Correct!");
		return PAM_SUCCESS;
	}

	// Clean up riddle memory
	free(riddle);
	return PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_IGNORE;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_SUCCESS;
}
#endif

#ifdef TEST
int main(int argc, char **argv)
{
	int found = 0;
	int length = 0;
	int attempts = 0;
	char buffer[MAX_INPUT] = {0};
	struct Riddle_t *riddle;
	printf("Getting Riddle\n");
	riddle = get_riddle();
	if (riddle == 0) {
		printf("ERROR GETTING RIDDLE\n");
		exit(-1);
	}
	printf("Got Riddle\n");
	
	// Get length of Answer
	length = strlen(riddle->Answer);

	// Loop attempts
	for(int attempts = 0; attempts < MAX_ATTEMPTS; attempts++) {
		printf("%s\n",riddle->Riddle);
		if(fgets(buffer, MAX_INPUT, stdin) == 0) {
			printf("ERROR READING INPUT\n");
			return PAM_AUTH_ERR;
		}
		if ( strncmp(buffer,riddle->Answer, length -1) == 0 ) {
			found = 1;
			break;
		}
	}
	if(found) {
		printf("Correct!\n");
	}
	exit(found);
}
#endif
