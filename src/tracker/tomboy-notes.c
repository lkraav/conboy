
#include "config.h"

#ifdef HILDON_HAS_APP_MENU



#include "tracker-module.h"

#define METADATA_FILE_NAME         "File:Name"
#define METADATA_NOTE_TITLE        "Note:Title"
#define METADATA_NOTE_TEXT         "Note:Text"
#define METADATA_NOTE_TAGS         "Note:Tags"
#define METADATA_NOTE_CREATE_DATE  "Note:CreateDate"
#define METADATA_NOTE_CHANGE_DATE  "Note:ChangeDate"


/* TODO: Define xml tags or better take from existing file */


#define TRACKER_TYPE_TOMBOY_FILE    (tracker_tomboy_file_get_type())
#define TRACKER_TOMBOY_FILE(module) (G_TYPE_CHECK_INSTANCE_CAST((module), TRACKER_TYPE_TOMBOY_FILE, TrackerTomboyFile))

typedef struct TrackerTomboyFile TrackerTomboyFile;
typedef struct TrackerTomboyFileClass TrackerTomboyFileClass;

struct TrackerTomboyFile {
	TrackerModuleFile parent_instance;
};

struct TrackerTomboyFileClass {
	TrackerModuleFileClass parent_class;
};

static GType                   tracker_tomboy_file_get_type      (void) G_GNUC_CONST;
static TrackerModuleMetadata * tracker_tomboy_file_get_metadata  (TrackerModuleFile *file);
static gchar *                 tracker_tomboy_file_get_text      (TrackerModuleFile *file);

G_DEFINE_DYNAMIC_TYPE (TrackerTomboyFile, tracker_tomboy_file, TRACKER_TYPE_MODULE_FILE);

/**
 * Override the methods get_metadata and get_text with our own implementation.
 */
static void
tracker_tomboy_file_class_init(TrackerTomboyFileClass *klass)
{
	g_debug("Conboy: Init class \n");

	TrackerModuleFileClass *file_class = TRACKER_MODULE_FILE_CLASS (klass);
	file_class->get_metadata = tracker_tomboy_file_get_metadata;
	file_class->get_text = tracker_tomboy_file_get_text;
}

static void
tracker_tomboy_file_class_finalize(TrackerTomboyFileClass *klass)
{
	g_debug("Conboy: Finalize class \n");
}

static void
tracker_tomboy_file_init(TrackerTomboyFile *file)
{
	g_debug("Conboy: Init file \n");
}

/**
 * Return all text from note-content but without formatting tags etc.
 */
static gchar*
tracker_tomboy_file_get_text(TrackerModuleFile *file)
{
	g_debug("Conboy: get text \n");
	return "Hallo Conboy";
}

/**
 * Return all  metadata like title, change date, etc.
 */
static TrackerModuleMetadata*
tracker_tomboy_file_get_metadata(TrackerModuleFile *file)
{
	g_debug("Conboy: get metadata \n");

	TrackerModuleMetadata *metadata;
	metadata = tracker_module_metadata_new();
	tracker_module_metadata_add_string(metadata, METADATA_NOTE_TITLE, "Conboy Rules");

	return metadata;
}

/* Boiler plate */
void
indexer_module_initialize(GTypeModule *module)
{
	g_debug("Conboy: init module \n");
	tracker_tomboy_file_register_type(module);
}

void
indexer_module_shutdown (void)
{
	g_debug("Conboy: shutdown module \n");
}

/* Boiler plate */
TrackerModuleFile *
indexer_module_create_file(GFile *file)
{
	g_debug("Conboy: create file \n");

	return g_object_new(TRACKER_TYPE_TOMBOY_FILE,
                        "file", file,
                        NULL);
}

#endif
