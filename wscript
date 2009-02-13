#!/usr/bin/env python
import os
srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'
VERSION_MAJOR_MINOR = ".".join(VERSION.split('.')[0:2])
APPNAME = 'krigsavers'

def build(bld):
    print ("Entering into directory " + os.getcwd())
    epoch = bld.new_task_gen('cc', 'cprogram')
    epoch.source = 'epochsaver.c gs-theme-window.c'
    epoch.target = 'epochsaver'
    epoch.includes = '.'
    epoch.uselib = 'GTK+-2.0 CAIRO'
    epoch.install_path = bld.env['SAVER']+'/gnome-screensaver'
    epoch.chmod = 0755

    bld.install_as('${DATADIR}/applications/screensavers/epochsaver.desktop', 'data/epochsaver.desktop')

def configure(conf):
    conf.check_tool('gcc')
    #conf.check_cfg(package='glib-2.0', mandatory=1, args='--cflags --libs')
    #conf.check_cfg(package='gobject-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='gtk+-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='cairo', args='--cflags --libs', mandatory=1)
    conf.env['SAVER'] = conf.check_cfg(package='gnome-screensaver', args='--variable=libexecdir', mandatory=1).strip()

def set_options(_):
    pass

