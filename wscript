#!/usr/bin/env python
import os
srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'
VERSION_MAJOR_MINOR = ".".join(VERSION.split('.')[0:2])
APPNAME = 'krigsavers'
SAVERS = ['epochsaver', 'circlesaver']

def build(bld):
    #print ("Entering into directory " + os.getcwd())

    for sav in SAVERS:
        bld.install_files('${PREFIX}/share/applications/screensavers', sav+'.desktop')
        saver = bld.new_task_gen('cc', 'cprogram')
        saver.source = sav+'.c gs-theme-window.c'
        saver.target = sav
        saver.includes = '.'
        saver.uselib = 'GTK+-2.0 CAIRO'
        saver.install_path = os.path.join(bld.env['SAVER'], 'gnome-screensaver')
        saver.chmod = 0755

    # variation of epochsaver
    bld.install_files('${PREFIX}/share/applications/screensavers', 'epochcountdown.desktop')


def configure(conf):
    conf.check_tool('gcc')
    conf.check_tool('g++')
    conf.check_cfg(package='glib-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='gobject-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='gtk+-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='cairo', args='--cflags --libs', mandatory=1)
    conf.env['SAVER'] = conf.check_cfg(package='gnome-screensaver',
                                       args='--variable=libexecdir',
                                       mandatory=1).strip()

def set_options(_):
    pass

