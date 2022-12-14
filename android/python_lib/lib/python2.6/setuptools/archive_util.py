"""Utilities for extracting common archive formats"""


__all__ = [
    "unpack_archive", "unpack_zipfile", "unpack_tarfile", "default_filter",
    "UnrecognizedFormat", "extraction_drivers", "unpack_directory",
]

import zipfile, tarfile, os, shutil
from pkg_resources import ensure_directory
from distutils.errors import DistutilsError

class UnrecognizedFormat(DistutilsError):
    """Couldn't recognize the archive type"""

def default_filter(src,dst):
    """The default progress/filter callback; returns True for all files"""   
    return dst























def unpack_archive(filename, extract_dir, progress_filter=default_filter,
    drivers=None
):
    """Unpack `filename` to `extract_dir`, or raise ``UnrecognizedFormat``

    `progress_filter` is a function taking two arguments: a source path
    internal to the archive ('/'-separated), and a filesystem path where it
    will be extracted.  The callback must return the desired extract path
    (which may be the same as the one passed in), or else ``None`` to skip
    that file or directory.  The callback can thus be used to report on the
    progress of the extraction, as well as to filter the items extracted or
    alter their extraction paths.

    `drivers`, if supplied, must be a non-empty sequence of functions with the
    same signature as this function (minus the `drivers` argument), that raise
    ``UnrecognizedFormat`` if they do not support extracting the designated
    archive type.  The `drivers` are tried in sequence until one is found that
    does not raise an error, or until all are exhausted (in which case
    ``UnrecognizedFormat`` is raised).  If you do not supply a sequence of
    drivers, the module's ``extraction_drivers`` constant will be used, which
    means that ``unpack_zipfile`` and ``unpack_tarfile`` will be tried, in that
    order.
    """
    for driver in drivers or extraction_drivers:
        try:
            driver(filename, extract_dir, progress_filter)
        except UnrecognizedFormat:
            continue
        else:
            return
    else:
        raise UnrecognizedFormat(
            "Not a recognized archive type: %s" % filename
        )







def unpack_directory(filename, extract_dir, progress_filter=default_filter):
    """"Unpack" a directory, using the same interface as for archives

    Raises ``UnrecognizedFormat`` if `filename` is not a directory
    """
    if not os.path.isdir(filename):
        raise UnrecognizedFormat("%s is not a directory" % (filename,))

    paths = {filename:('',extract_dir)}
    for base, dirs, files in os.walk(filename):
        src,dst = paths[base]
        for d in dirs:
            paths[os.path.join(base,d)] = src+d+'/', os.path.join(dst,d)
        for f in files:
            name = src+f
            target = os.path.join(dst,f)
            target = progress_filter(src+f, target)
            if not target:
                continue    # skip non-files
            ensure_directory(target)
            f = os.path.join(base,f)
            shutil.copyfile(f, target)
            shutil.copystat(f, target)


















def unpack_zipfile(filename, extract_dir, progress_filter=default_filter):
    """Unpack zip `filename` to `extract_dir`

    Raises ``UnrecognizedFormat`` if `filename` is not a zipfile (as determined
    by ``zipfile.is_zipfile()``).  See ``unpack_archive()`` for an explanation
    of the `progress_filter` argument.
    """

    if not zipfile.is_zipfile(filename):
        raise UnrecognizedFormat("%s is not a zip file" % (filename,))

    z = zipfile.ZipFile(filename)
    try:
        for info in z.infolist():
            name = info.filename

            # don't extract absolute paths or ones with .. in them
            if name.startswith('/') or '..' in name:
                continue

            target = os.path.join(extract_dir, *name.split('/'))
            target = progress_filter(name, target)
            if not target:
                continue
            if name.endswith('/'):
                # directory
                ensure_directory(target)
            else:
                # file
                ensure_directory(target)
                data = z.read(info.filename)
                f = open(target,'wb')
                try:
                    f.write(data)
                finally:
                    f.close()
                    del data
    finally:
        z.close()


def unpack_tarfile(filename, extract_dir, progress_filter=default_filter):
    """Unpack tar/tar.gz/tar.bz2 `filename` to `extract_dir`

    Raises ``UnrecognizedFormat`` if `filename` is not a tarfile (as determined
    by ``tarfile.open()``).  See ``unpack_archive()`` for an explanation
    of the `progress_filter` argument.
    """

    try:
        tarobj = tarfile.open(filename)
    except tarfile.TarError:
        raise UnrecognizedFormat(
            "%s is not a compressed or uncompressed tar file" % (filename,)
        )

    try:
        tarobj.chown = lambda *args: None   # don't do any chowning!
        for member in tarobj:
            if member.isfile() or member.isdir():
                name = member.name
                # don't extract absolute paths or ones with .. in them
                if not name.startswith('/') and '..' not in name:
                    dst = os.path.join(extract_dir, *name.split('/'))                
                    dst = progress_filter(name, dst)
                    if dst:
                        if dst.endswith(os.sep):
                            dst = dst[:-1]
                        try:
                            tarobj._extract_member(member,dst)  # XXX Ugh
                        except tarfile.ExtractError:
                            pass    # chown/chmod/mkfifo/mknode/makedev failed
        return True
    finally:
        tarobj.close()




extraction_drivers = unpack_directory, unpack_zipfile, unpack_tarfile


