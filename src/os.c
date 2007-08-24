/*
** 2005 November 29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains OS interface code that is common to all
** architectures.
*/
#define _SQLITE_OS_C_ 1
#include "sqliteInt.h"
#undef _SQLITE_OS_C_

/*
** The following routines are convenience wrappers around methods
** of the sqlite3_file object.  This is mostly just syntactic sugar. All
** of this would be completely automatic if SQLite were coded using
** C++ instead of plain old C.
*/
int sqlite3OsClose(sqlite3_file *pId){
  int rc = SQLITE_OK;
  if( pId->pMethods ){
    rc = pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
  return rc;
}
int sqlite3OsRead(sqlite3_file *id, void *pBuf, int amt, i64 offset){
  return id->pMethods->xRead(id, pBuf, amt, offset);
}
int sqlite3OsWrite(sqlite3_file *id, const void *pBuf, int amt, i64 offset){
  return id->pMethods->xWrite(id, pBuf, amt, offset);
}
int sqlite3OsTruncate(sqlite3_file *id, i64 size){
  return id->pMethods->xTruncate(id, size);
}
int sqlite3OsSync(sqlite3_file *id, int flags){
  return id->pMethods->xSync(id, flags);
}
int sqlite3OsFileSize(sqlite3_file *id, i64 *pSize){
  return id->pMethods->xFileSize(id, pSize);
}
int sqlite3OsLock(sqlite3_file *id, int lockType){
  return id->pMethods->xLock(id, lockType);
}
int sqlite3OsUnlock(sqlite3_file *id, int lockType){
  return id->pMethods->xUnlock(id, lockType);
}
int sqlite3OsBreakLock(sqlite3_file *id){
  return id->pMethods->xBreakLock(id);
}
int sqlite3OsCheckReservedLock(sqlite3_file *id){
  return id->pMethods->xCheckReservedLock(id);
}

#ifdef SQLITE_TEST
  /* The following two variables are used to override the values returned
  ** by the xSectorSize() and xDeviceCharacteristics() vfs methods for
  ** testing purposes. They are usually set by a test command implemented
  ** in test6.c.
  */
  int sqlite3_test_sector_size = 0;
  int sqlite3_test_device_characteristics = 0;
  int sqlite3OsDeviceCharacteristics(sqlite3_file *id){
    int dc = id->pMethods->xDeviceCharacteristics(id);
    return dc | sqlite3_test_device_characteristics;
  }
  int sqlite3OsSectorSize(sqlite3_file *id){
    if( sqlite3_test_sector_size==0 ){
      int (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
      return (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
    }
    return sqlite3_test_sector_size;
  }
#else
  int sqlite3OsSectorSize(sqlite3_file *id){
    int (*xSectorSize)(sqlite3_file*) = id->pMethods->xSectorSize;
    return (xSectorSize ? xSectorSize(id) : SQLITE_DEFAULT_SECTOR_SIZE);
  }
  int sqlite3OsDeviceCharacteristics(sqlite3_file *id){
    return id->pMethods->xDeviceCharacteristics(id);
  }
#endif

#if defined(SQLITE_TEST) || defined(SQLITE_DEBUG)
  /* These methods are currently only used for testing and debugging. */
  int sqlite3OsFileHandle(sqlite3_file *id){
    /* return id->pMethods->xFileHandle(id); */
    return 0;
  }
  int sqlite3OsLockState(sqlite3_file *id){
    return id->pMethods->xLockState(id);
  }
#endif

/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
int sqlite3OsOpen(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  sqlite3_file *pFile, 
  int flags, 
  int *pFlagsOut
){
  return pVfs->xOpen(pVfs, zPath, pFile, flags, pFlagsOut);
}
int sqlite3OsDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
  return pVfs->xDelete(pVfs, zPath, dirSync);
}
int sqlite3OsAccess(sqlite3_vfs *pVfs, const char *zPath, int flags){
  return pVfs->xAccess(pVfs, zPath, flags);
}
int sqlite3OsGetTempName(sqlite3_vfs *pVfs, char *zBufOut){
  return pVfs->xGetTempName(pVfs, zBufOut);
}
int sqlite3OsFullPathname(sqlite3_vfs *pVfs, const char *zPath, char *zPathOut){
  return pVfs->xFullPathname(pVfs, zPath, zPathOut);
}
void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  return pVfs->xDlOpen(pVfs, zPath);
}
void sqlite3OsDlError(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
  pVfs->xDlError(pVfs, nByte, zBufOut);
}
void *sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHandle, const char *zSymbol){
  return pVfs->xDlSym(pVfs, pHandle, zSymbol);
}
void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  pVfs->xDlClose(pVfs, pHandle);
}
int sqlite3OsRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
  return pVfs->xRandomness(pVfs, nByte, zBufOut);
}
int sqlite3OsSleep(sqlite3_vfs *pVfs, int nMicro){
  return pVfs->xSleep(pVfs, nMicro);
}
int sqlite3OsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  return pVfs->xCurrentTime(pVfs, pTimeOut);
}

int sqlite3OsOpenMalloc(
  sqlite3_vfs *pVfs, 
  const char *zFile, 
  sqlite3_file **ppFile, 
  int flags,
  int *pOutFlags
){
  int rc = SQLITE_NOMEM;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3_malloc(pVfs->szOsFile);
  if( pFile ){
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    if( rc!=SQLITE_OK ){
      sqlite3_free(pFile);
    }else{
      *ppFile = pFile;
    }
  }
  return rc;
}
int sqlite3OsCloseFree(sqlite3_file *pFile){
  int rc = SQLITE_OK;
  if( pFile ){
    rc = sqlite3OsClose(pFile);
    sqlite3_free(pFile);
  }
  return rc;
}

/*
** The list of all registered VFS implementations.  This list is
** initialized to the single VFS returned by sqlite3OsDefaultVfs()
** upon the first call to sqlite3_vfs_find().
*/
static sqlite3_vfs *vfsList = 0;

/*
** Locate a VFS by name.  If no name is given, simply return the
** first VFS on the list.
*/
sqlite3_vfs *sqlite3_vfs_find(const char *zVfs){
  sqlite3_mutex *mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER);
  sqlite3_vfs *pVfs;
  static int isInit = 0;
  sqlite3_mutex_enter(mutex);
  if( !isInit ){
    vfsList = sqlite3OsDefaultVfs();
    isInit = 1;
  }
  for(pVfs = vfsList; pVfs; pVfs=pVfs->pNext){
    if( zVfs==0 ) break;
    if( strcmp(zVfs, pVfs->zName)==0 ) break;
  }
  if( pVfs ){
    pVfs->nRef++;
    assert( pVfs->nRef==1 || pVfs->vfsMutex!=0 );
    assert( pVfs->nRef>1 || pVfs->vfsMutex==0 );
    if( pVfs->vfsMutex==0 ){
      pVfs->vfsMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
    }
  }
  sqlite3_mutex_leave(mutex);
  return pVfs;
}

/*
** Release a VFS once it is no longer needed.
*/
int sqlite3_vfs_release(sqlite3_vfs *pVfs){
  sqlite3_mutex *mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER);
  sqlite3_mutex_enter(mutex);
  assert( pVfs->nRef>0 );
  pVfs->nRef--;
  if( pVfs->nRef==0 && pVfs->vfsMutex ){
    sqlite3_mutex_free(pVfs->vfsMutex);
    pVfs->vfsMutex = 0;
  }
  sqlite3_mutex_leave(mutex);
  return SQLITE_OK;
}

/*
** Unlink a VFS from the linked list
*/
static void vfsUnlink(sqlite3_vfs *pVfs){
  assert( sqlite3_mutex_held(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER)) );
  if( vfsList==pVfs ){
    vfsList = pVfs->pNext;
  }else{
    sqlite3_vfs *p = vfsList;
    while( p->pNext && p->pNext!=pVfs ){
      p = p->pNext;
    }
    if( p->pNext==pVfs ){
      p->pNext = pVfs->pNext;
    }
  }
}

/*
** Register a VFS with the system.  It is harmless to register the same
** VFS multiple times.  The new VFS becomes the default if makeDflt is
** true.
*/
int sqlite3_vfs_register(sqlite3_vfs *pVfs, int makeDflt){
  sqlite3_mutex *mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER);
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  if( makeDflt || vfsList==0 ){
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  }else{
    pVfs->pNext = vfsList->pNext;
    pVfs->pNext = pVfs;
  }
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  return SQLITE_OK;
}

/*
** Unregister a VFS so that it is no longer accessible.
*/
int sqlite3_vfs_unregister(sqlite3_vfs *pVfs){
  sqlite3_mutex *mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER);
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  assert(vfsList);
  sqlite3_mutex_leave(mutex);
  return SQLITE_OK;
}
