
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <syslog.h>
#include <planet/filesystem.hpp>

#define s(str) std::string(str)
#define LOG_EXCEPTION_MSG(e) \
    BOOST_LOG_TRIVIAL(error) << __func__ << ": " << (e).what();

static Ixp9Srv srv;
std::shared_ptr<planet::filesystem> fs;

class planet_fid {
public:
    template<typename ...Args>
    static planet_fid *create(Args&& ...args)
    {
        return new planet_fid(std::forward<Args>(args)...);
    }

    static void destroy(void *fid)
    {
        delete (planet_fid *)fid;
    }

    std::string path;
    planet::shared_ptr<planet::fs_entry> entry;
    planet::handle_t handle = -1;
    bool first = true;

private:
    planet_fid(std::string p)
    {
        if (p != "/" && p.back() == '/')
            p.pop_back();
        path = std::move(p);
        entry = fs->root()->get_entry_of(path);
    }
};

static void
throw_system_error(int err)
{
    throw std::system_error(err, std::system_category());
}

struct stat getattr(std::string const& path)
{
    struct stat stbuf;
    std::memset(&stbuf, 0, sizeof (struct stat));
    fs->root()->getattr(path, stbuf);
    return stbuf;
}

static void
stat_posix_to_9p(IxpStat *stat, std::string name, struct stat *buf)
{
    stat->type = 0;
    stat->dev = 0;
    stat->qid.type = buf->st_mode & S_IFMT;
    stat->qid.path = buf->st_ino;
    stat->qid.version = 0;
    stat->mode = buf->st_mode & 0777;
    if (S_ISDIR(buf->st_mode)) {
        stat->mode |= P9_DMDIR;
        stat->qid.type |= P9_QTDIR;
    }
    stat->atime = buf->st_atime;
    stat->mtime = buf->st_mtime;
    stat->length = buf->st_size;
    stat->name = strdup(name.c_str());
    stat->uid  = getenv("USER");
    stat->gid  = getenv("USER");
    stat->muid = getenv("USER");
}

static std::string
strerrno(int err)
{
    return std::error_code(err, std::system_category()).message();
}

static void
planet_attach(Ixp9Req *r)
{
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": New 9P client attached: uname="
        << r->ifcall.tattach.uname << " aname=" << r->ifcall.tattach.aname;

    int ret = 0;
    try {
        r->fid->qid.path = getattr("/").st_ino;
        r->fid->qid.type = P9_QTDIR;
        r->fid->aux = planet_fid::create("/");
        r->ofcall.rattach.qid = r->fid->qid;
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_clunk(Ixp9Req *r)
{
    auto *fid = (planet_fid *)r->fid->aux;
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << fid->path;

    int ret = 0;
    try {
        if (fid->handle >= 0)
            fs->root()->close(fid->handle);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_create(Ixp9Req *r)
{
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;

        assert(fid->entry->type() == planet::file_type::directory);

        std::string path = fid->path
            + (fid->path == "/" ? "" : "/")
            + r->ifcall.tcreate.name;

        if (r->ifcall.tcreate.perm & P9_DMDIR) {
            BOOST_LOG_TRIVIAL(trace) << __func__ << ": mkdir: fid=" << fid->path << " path=" << path;
            fs->root()->mkdir(path, r->ifcall.tcreate.perm);
        } else {
            BOOST_LOG_TRIVIAL(trace) << __func__ << ": mknod: fid=" << fid->path << " path=" << path;
            fs->root()->mknod(path, r->ifcall.tcreate.perm);
            fid->handle = fs->root()->open(path);
        }

        fid->entry = fs->root()->get_entry_of(path);
        auto stbuf = getattr(path);
        r->fid->qid.path = stbuf.st_ino;
        r->fid->qid.type = stbuf.st_mode & S_IFMT;

    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_flush(Ixp9Req *r)
{
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": called";
    ixp_respond(r, nullptr);
}

static void
planet_open(Ixp9Req *r)
{
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;
        BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << fid->path;

        if (fid->entry->type() != planet::file_type::directory) {
            fid->handle = fs->root()->open(fid->path);
        }

        struct stat stbuf = getattr(fid->path);
        r->fid->qid.path = stbuf.st_ino;
        r->fid->qid.type = stbuf.st_mode & S_IFMT;

    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_read_dir(Ixp9Req *r)
{
    auto *fid = (planet_fid *)r->fid->aux;
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": directory path=" << fid->path;

    char *buf = (char *)ixp_emallocz(r->ifcall.tread.count);
    IxpMsg m = ixp_message(buf, r->ifcall.tread.count, MsgPack);

    int count = 0;
    for (auto const& entry_name : fs->root()->readdir(fid->path)) {
        if (!fid->first)
            break;

        std::string path = fid->path
            + (fid->path == "/" ? "" : "/")
            + entry_name;
        auto stbuf = getattr(path);

        IxpStat s;
        stat_posix_to_9p(&s, entry_name, &stbuf);

        if ((count += ixp_sizeof_stat(&s)) > r->ifcall.tread.count)
            break;

        // Pack the stat to the binary
        ixp_pstat(&m, &s);
    }
    r->ofcall.rread.count = fid->first ? count : 0;
    r->ofcall.rread.data = (char*)m.data;
    fid->first = false;
}

static void
planet_read_file(Ixp9Req *r)
{
    auto *fid = (planet_fid *)r->fid->aux;
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": file path=" << fid->path;

    r->ofcall.rread.data = (char *)ixp_emallocz(r->ifcall.tread.count);
    if (!r->ofcall.rread.data)
        return;

    int count = fs->root()->read(
            fid->handle,
            r->ofcall.rread.data,
            r->ifcall.tread.count,
            r->ifcall.tread.offset);
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": read_count=" << count;
    if (count < 0)
        throw_system_error(EPERM);

    r->ofcall.rread.count = count;
}

static void
planet_read(Ixp9Req *r)
{
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;
        BOOST_LOG_TRIVIAL(trace) << __func__ << ": offset="
            << r->ifcall.tread.offset << " count=" << r->ifcall.tread.count;

        if (fid->entry->type() == planet::file_type::directory)
            planet_read_dir(r);
        else
            planet_read_file(r);

    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_remove(Ixp9Req *r)
{
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;
        BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << fid->path;

        if (fid->entry->type() == planet::file_type::directory)
            fs->root()->rmdir(fid->path, true);
        else
            fs->root()->unlink(fid->path, true);

    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_stat(Ixp9Req *r)
{
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;
        BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << fid->path;

        IxpStat s;
        struct stat stbuf = getattr(fid->path);
        stat_posix_to_9p(&s, fid->entry->name(), &stbuf);

        // Pack the stat to the binary
        int size = ixp_sizeof_stat(&s);
        char *buf = (char *)ixp_emallocz(size);
        IxpMsg m = ixp_message(buf, size, MsgPack);
        ixp_pstat(&m, &s);

        r->fid->qid = s.qid;
        r->ofcall.rstat.nstat = size;
        r->ofcall.rstat.stat = (uint8_t *)m.data;

    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_walk(Ixp9Req *r)
{
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;
        std::string path = (fid->path == "/" ? "" : fid->path);
        BOOST_LOG_TRIVIAL(trace) << __func__ << ": fid=" << fid->path << " nwname=" << r->ifcall.twalk.nwname;

        int i = 0;
        for (; i < r->ifcall.twalk.nwname; ++i) {
            path += s("/") + r->ifcall.twalk.wname[i];
            BOOST_LOG_TRIVIAL(trace) << __func__ << ": i=" << i << " path=" << path;

            struct stat stbuf;
            std::memset(&stbuf, 0, sizeof (struct stat));
            try {
                fs->root()->getattr(path, stbuf);
            } catch (std::system_error& e) {
                LOG_EXCEPTION_MSG(e);
                break;
            }

            r->ofcall.rwalk.wqid[i].type = stbuf.st_mode & S_IFMT;
            r->ofcall.rwalk.wqid[i].path = stbuf.st_ino;
        }

        r->newfid->aux = r->ifcall.twalk.nwname ?
            planet_fid::create(path) : planet_fid::create(*fid);
        r->ofcall.rwalk.nwqid = i;

    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_write(Ixp9Req *r)
{
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;
        //BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << fid->path;

        if (fid->entry->type() != planet::file_type::directory) {
            int count = fs->root()->write(fid->handle,
                    r->ifcall.twrite.data,
                    r->ifcall.twrite.count,
                    r->ifcall.twrite.offset);
            if (count < 0) {
                ret = EPERM;
            } else {
                r->ofcall.rwrite.count = count;
            }
        } else {
            ret = EISDIR;
        }
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_wstat(Ixp9Req *r)
{
    syslog(LOG_NOTICE, "%s: called", "planet_wstat");
    int ret = 0;
    try {
        auto *fid = (planet_fid *)r->fid->aux;
        BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << fid->path;
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = EIO;
    }
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": ret=" << ret << " errstr=" << (ret ? strerrno(ret).c_str() : "none");
    ixp_respond(r, ret ? strerrno(ret).c_str() : nullptr);
}

static void
planet_freefid(IxpFid *f)
{
    auto *fid = (planet_fid *)f->aux;
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << fid->path;

    planet_fid::destroy(fid);
}

static void
planetfs_create_initial_fs_structure()
{
    auto create_file =
        [](std::string const& path, const char *buf, int len)
        {
            fs->root()->mknod(path, 0777);
            auto handle = fs->root()->open(path);
            fs->root()->write(handle, buf, len, 0);
            fs->root()->close(handle);
        };

    std::string buf = "The dummy content you will see\n";
    create_file("/file1", buf.data(), buf.length());

    std::string buf2 = "\n\n\nhello";
    create_file("/file2", buf2.data(), buf2.length());

    fs->root()->mkdir("/dir1", 0777);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " proto!addr[!port]" << std::endl
                  << "Examples: " << argv[0] << " unix!mysrv" << std::endl
                  << "          " << argv[0] << " tcp!localhost!564" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    try {
        fs = std::make_shared<planet::filesystem>(S_IRWXU);
        planetfs_create_initial_fs_structure();

        srv.attach  = planet_attach;
        srv.clunk   = planet_clunk;
        srv.create  = planet_create;
        srv.flush   = planet_flush;
        srv.open    = planet_open;
        srv.read    = planet_read;
        srv.remove  = planet_remove;
        srv.stat    = planet_stat;
        srv.walk    = planet_walk;
        srv.write   = planet_write;
        srv.wstat   = planet_wstat;
        srv.freefid = planet_freefid;

        fs->listen(argc, argv, srv, nullptr);
        fs->start_main();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    std::exit(EXIT_SUCCESS);
}
