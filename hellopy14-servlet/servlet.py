import sys,os
sys.stdout = sys.stderr
sys.path.append(os.path.dirname(__file__))

localDir = os.path.dirname(__file__)
absDir = os.path.join(os.getcwd(), localDir)

import atexit
import threading
import cherrypy
import subprocess

from os import listdir

#cherrypy.config.update({'environment': 'production'})
cherrypy.config.update({'environment': 'test_suite'})

if cherrypy.__version__.startswith('3.0') and cherrypy.engine.state == 0:
  cherrypy.engine.start(blocking=False)
  atexit.register(cherrypy.engine.stop)

class FileDemo(object):
  # _cp_config = {
  #   'tools.staticdir.on': True,
  # }
  
  def index(self):
    retval = """
    <html><body>
      <h2>Upload a file</h2>
      <form action="upload" method="post" enctype="multipart/form-data">
      filename: <input type="file" name="file_upload" /><br />
      <input type="submit" />
      </form>
      <h2>Existing files:</h2>
      %s
    </body></html>
    """
    filelist = os.listdir(os.path.join(absDir,'uploads/'))
    cherrypy.log('aaaa')
    filelinks = ''
    for f in filelist:
      filelinks = filelinks + ("<a href='delete/?filename=%s'>[x]</a> "
                               "<a href='preview/?filename=%s'>[v]</a> "
                               "<a href='download/?filename=%s'>%s</a> <br/>" % (f,f,f,f))
    retval = retval % filelinks

    return retval;
  index.exposed = True

  def upload(self, file_upload):
    out = """<html>
    <body onload="setTimeout(function(){window.location = \'/\';},1500);">
      File length: %s<br />
      File filename: %s<br />
      File mime-type: %s<br />
      File content: 
    </body>
    </html>"""

    size = 0
    fullname = os.path.join(absDir,'uploads/%s' % file_upload.filename)
    fdst = open(fullname,'wt')
    while True:
      data = file_upload.file.read(8192)
      if not data:
        break
      size += len(data)
      fdst.write(data)
    fdst.close()

    return out % (size, file_upload.filename, file_upload.content_type)
  upload.exposed = True

  def download(self,filename):
    path = os.path.join(absDir, 'uploads/%s' % filename)
    return cherrypy.lib.static.serve_file(path, "application/x-download",
                 "attachment", os.path.basename(path))
  download.exposed = True

  def preview(self,filename):
    out = """<html>
    <body>
      %s
    </body>
    </html>"""
    content = ""
    filepath = os.path.join(absDir, 'uploads/%s' % filename)
    with open(filepath) as fp:
      for lines in fp.readlines():
        content = content + '<pre>' + lines + '</pre>'
    return out % content
  preview.exposed = True

  def delete(self,filename):
    out = """<html>
    <body onload="setTimeout(function(){window.location = \'/\';},1500);">
      %s
    </body>
    </html>"""
    uploadpath = os.path.join(absDir, 'uploads/%s' % filename)
    trashpath = os.path.join(absDir, 'trash/%s' % filename)
    try:
      os.renames(uploadpath,trashpath);
      return out % ("File %s removed." % filename)
    except OSError:
      return out % ("File %s not found." % filename)
    
  delete.exposed = True

application = cherrypy.Application(FileDemo(), script_name=None, config=None)

