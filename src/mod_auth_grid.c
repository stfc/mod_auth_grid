#include <ctype.h>

#include "apr.h"
#include "apr_strings.h"
#include "apr_tables.h"

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"


typedef struct {
	const char *gridmapfile;
	const char *user_env_in;
	const char *user_env_out;
} auth_grid_dir_config;

module AP_MODULE_DECLARE_DATA auth_grid_module;


/*
 * Parses a line of the grid-mapfile to find the DN and the first username,
 * and NUL-terminates them if found.
 */
static int
parse_line(apr_file_t *file, char buf[1024], char **dn, char **username)
{
	*dn = NULL;
	*username = NULL;

	unsigned int i = 0;

	if (apr_file_gets(buf, 1024, file) != APR_SUCCESS)
		return EOF;

	// skip leading whitespace
	for ( ; i < 1024; ++i)
		if (!isspace(buf[i]))
			break;

	// ignore lines beginning with '#' and empty lines
	if ('#' == buf[i] || '\0' == buf[i])
		return 0;

	// get DN
	if ('"' == buf[i]) {
		// quoted
		*dn = &buf[++i];

		for ( ; i < 1024; ++i)
			if ('"' == buf[i])
				break;
	} else {
		// unquoted
		*dn = &buf[i++];

		for ( ; i < 1024; ++i)
			if (isspace(buf[i]))
				break;
	}

	buf[i++] = '\0';

	// skip whitespace after DN
	for ( ; i < 1024; ++i)
		if (!isspace(buf[i]))
			break;

	// get username
	*username = &buf[i];

	for ( ; i < 1024; ++i)
		if (isspace(buf[i]) || ',' == buf[i])
			break;

	buf[i] = '\0';

	return 0;
}

/*
 * Find the fisrt line in the grid-mapfile that matches the DN and has a
 * username.
 */
static char *
get_username(apr_pool_t *p, apr_file_t *file, const char *user_dn)
{
	char  buf[1024];
	char *dn;
	char *username;
	char *sav_username = NULL;

	int dn_len;
	int user_dn_len;

	user_dn_len = strlen(user_dn);

	while (parse_line(file, buf, &dn, &username) != EOF) {
		if (dn != NULL && username != NULL) {
			// attempt to match a normal DN
			if (strcasecmp(dn, user_dn) == 0) {
				sav_username = apr_pstrdup(p, username);
				break;
			}

			// attempt to match a proxy DN
			dn_len = strlen(dn);
			if (user_dn_len > dn_len
			 && strncasecmp(dn, user_dn, dn_len) == 0
			 && '/' == user_dn[dn_len]) {
				sav_username = apr_pstrdup(p, username);
				break;
			}
		}
	}

	return sav_username;
}

/*
 * Dummy authentication function. This is for when mod_ssl is used with the
 * `SSLUserName' directive to set `r->user' to the user's DN. In that case, we
 * need to handle the `check_user_id' hook so that the `auth_checker' hook
 * will fire to run the actual grid-mapfile lookup.
 */
static int auth_grid(request_rec *r)
{
	const char *current_auth;

	// Only handle this if `AuthType Grid' is set
	current_auth = ap_auth_type(r);
	if (!current_auth || strcasecmp(current_auth, "Grid"))
		return DECLINED;

	if (!r->user)
		return DECLINED;

	r->ap_auth_type = "Grid";

	return OK;
}

/*
 * This does the actual gridmap-file lookup. It needs to impement the
 * `auth_checker' hook, not the `check_user_id' hook, because `r->user' may be
 * set by an actual auth provider and not by mod_ssl. Since it changes
 * `r->user', it must be run before any other authorisation modules.
 */
static int auth_gridmapfile(request_rec *r)
{
	const char *dn;
	const char *gridmapfile;
	const char *user_env_in;
	const char *user_env_out;

	apr_file_t   *file = NULL;
	apr_status_t  status;

	auth_grid_dir_config *conf;

	conf = ap_get_module_config(r->per_dir_config, &auth_grid_module);

	// If no grid-mapfile specified, then don't use this module
	gridmapfile = conf->gridmapfile;
	if (NULL == gridmapfile)
		return DECLINED;

	// If `AuthGridUserEnvIn' specified, read the DN from it, otherwise
	// use `user' from the request record
	user_env_in = conf->user_env_in;
	if (NULL == user_env_in) {
		dn = r->user;
		if (NULL == dn)
			return DECLINED;
	} else {
		dn = apr_table_get(r->subprocess_env, user_env_in);
		if (NULL == dn) {
			ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
			              "Cannot find variable in subprocess"
			              " environment table: %s", user_env_in);
			return HTTP_INTERNAL_SERVER_ERROR;
		}
	}

	// If `AuthGridUserEnvOut' specified, write the DN to it
	user_env_out = conf->user_env_out;
	if (user_env_out != NULL)
		apr_table_setn(r->subprocess_env, user_env_out, dn);

	// Open the grid-mapfile and get the username (if the DN is there)
	status = apr_file_open(&file, gridmapfile, APR_READ, APR_OS_DEFAULT,
	                       r->pool);
	if (status != APR_SUCCESS) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, status, r,
		              "Cannot open grid-mapfile: %s", gridmapfile);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	r->user = get_username(r->pool, file, dn);
	apr_file_close(file);

	if (NULL == r->user) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
		              "Cannot find user in grid-mapfile: %s", dn);

		return HTTP_FORBIDDEN;
	}

	// We've changed the user, now leave as if nothing's happened.
	return DECLINED;
}

static void *
create_dir_config(apr_pool_t *p, char *dir)
{
	return apr_pcalloc(p, sizeof(auth_grid_dir_config));
}

static const command_rec cmds[] = {
	AP_INIT_TAKE1("AuthGridMapfile", ap_set_string_slot,
	              (void *)APR_OFFSETOF(auth_grid_dir_config,
	                                   gridmapfile),
	              ACCESS_CONF, "Location of grid-mapfile"),

	AP_INIT_TAKE1("AuthGridUserEnvIn", ap_set_string_slot,
	              (void *)APR_OFFSETOF(auth_grid_dir_config,
	                                   user_env_in),
	              ACCESS_CONF, "Environment variable to read DN from"),

	AP_INIT_TAKE1("AuthGridUserEnvOut", ap_set_string_slot,
	              (void *)APR_OFFSETOF(auth_grid_dir_config,
	                                   user_env_out),
	              ACCESS_CONF, "Environment variable to write DN into"),

	{ NULL }
};

static void
register_hooks(apr_pool_t *p)
{
	ap_hook_check_user_id(auth_grid, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_auth_checker(auth_gridmapfile, NULL, NULL, APR_HOOK_FIRST);
}

module AP_MODULE_DECLARE_DATA auth_grid_module =
{
	STANDARD20_MODULE_STUFF,
	create_dir_config,
	NULL,                   // merge_dir_config
	NULL,                   // create_server_config
	NULL,                   // merge_server_config
	cmds,
	register_hooks
};
