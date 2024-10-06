/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "gst_ios_init.h"

void gst_ios_pre_init()
{
    const NSString *const resources = [[NSBundle mainBundle] resourcePath];
    const NSString *const tmp = NSTemporaryDirectory();
    const NSString *const cache = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Caches"];
    const NSString *const docs = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"];

    const gchar *const resources_dir = [resources UTF8String];
    const gchar *const tmp_dir = [tmp UTF8String];
    const gchar *const cache_dir = [cache UTF8String];
    const gchar *const docs_dir = [docs UTF8String];

    g_setenv("TMP", tmp_dir, TRUE);
    g_setenv("TEMP", tmp_dir, TRUE);
    g_setenv("TMPDIR", tmp_dir, TRUE);
    g_setenv("XDG_RUNTIME_DIR", resources_dir, TRUE);
    g_setenv("XDG_CACHE_HOME", cache_dir, TRUE);

    g_setenv("HOME", docs_dir, TRUE);
    g_setenv("XDG_DATA_DIRS", resources_dir, TRUE);
    g_setenv("XDG_CONFIG_DIRS", resources_dir, TRUE);
    g_setenv("XDG_CONFIG_HOME", cache_dir, TRUE);
    g_setenv("XDG_DATA_HOME", resources_dir, TRUE);
    g_setenv("FONTCONFIG_PATH", resources_dir, TRUE);

    gchar *const ca_certificates = g_build_filename(resources_dir, "ssl", "certs", "ca-certificates.crt", NULL);
    g_setenv("CA_CERTIFICATES", ca_certificates, TRUE);
    g_free(ca_certificates);
}
