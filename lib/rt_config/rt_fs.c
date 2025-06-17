#include "rt_fs.h"


//
//int rt_fs__setup_flash(struct fs_mount_t *mnt)
//{
//	int rc = 0;
//	unsigned int id;
//	const struct flash_area *pfa;
//
//	mnt->storage_dev = (void *)STORAGE_PARTITION_ID;
//	id = STORAGE_PARTITION_ID;
//
//	rc = flash_area_open(id, &pfa);
//	LOG_INF("Area %u at 0x%x on %s for %u kB",
//	       id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,
//	       ((unsigned int)pfa->fa_size / 1024));
//
////	if (rc < 0 && IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
////		LOG_INF("Erasing flash area ... ");
////		rc = flash_area_flatten(pfa, 0, pfa->fa_size);
////		LOG_INF("%d\n", rc);
////	}
//
//	if (rc < 0) {
//		flash_area_close(pfa);
//	}
//	return rc;
//}
//
//
//static int rt_fs__mount_app_fs(struct fs_mount_t *mnt)
//{
//	int rc;
//	static FATFS fat_fs;
//
//	mnt->type = FS_FATFS;
//	mnt->fs_data = &fat_fs;
//	mnt->mnt_point = "/NAND:";
//
//	rc = fs_mount(mnt);
//
//	return rc;
//}

int8_t rt_fs__init()
{
//    struct fs_mount_t *mp = &fs_mnt;
//
//	struct fs_mount_t *mp = &fs_mnt;
//	struct fs_dir_t dir;
//	struct fs_statvfs sbuf;
//	int rc;
//
//	fs_dir_t_init(&dir);
//
//	rc = rt_fs__setup_flash(mp);
//	if (rc < 0) {
//		LOG_ERR("Failed to setup flash area");
//		return -1;
//	}
//
//	rc = rt_fs__mount_app_fs(mp);
//	if (rc < 0) {
//		LOG_ERR("Failed to mount filesystem");
//		return -1;
//	}
//
//	/* Allow log messages to flush to avoid interleaved output */
//	k_sleep(K_MSEC(50));
//
//	LOG_INF("Mount %s: %d", fs_mnt.mnt_point, rc);
//
//	rc = fs_statvfs(mp->mnt_point, &sbuf);
//	if (rc < 0) {
//		LOG_ERR("FAIL: statvfs: %d\n", rc);
//		return -1;
//	}
//
//	rc = fs_opendir(&dir, mp->mnt_point);
//	LOG_INF("%s opendir: %d\n", mp->mnt_point, rc);
//
//	if (rc < 0) {
//		LOG_ERR("Failed to open directory");
//		return -1;
//	}
//
//	while (rc >= 0) {
//		struct fs_dirent ent = { 0 };
//
//		rc = fs_readdir(&dir, &ent);
//		if (rc < 0) {
//			LOG_ERR("Failed to read directory entries");
//			return -1;
//			break;
//		}
//		if (ent.name[0] == 0) {
//			break;
//		}
//		LOG_INF("\t%c\t %s\t  %d kB",
//		       (ent.type == FS_DIR_ENTRY_FILE) ? 'F' : 'D',
//			   ent.name, 
//			   (ent.size / 1024));
//	}
//
//	(void)fs_closedir(&dir);
	
	return 0;
}