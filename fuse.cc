/*
 * receive request from fuse and call methods of yfs_client
 *
 * started life as low-level example in the fuse distribution.  we
 * have to use low-level interface in order to get i-numbers.  the
 * high-level interface only gives us complete paths.
 */

//penis

#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include "yfs_client.h"

int myid;
yfs_client *yfs;

int id() { 
  return myid;
}

yfs_client::status
getattr(yfs_client::inum inum, struct stat &st)
{
  yfs_client::status ret;

  bzero(&st, sizeof(st));

  st.st_ino = inum;
  printf("getattr %016llx %d\n", inum, yfs->isfile(inum));
  if(yfs->isfile(inum)){
     yfs_client::fileinfo info;
     ret = yfs->getfile(inum, info);
     if(ret != yfs_client::OK)
       return ret;
     st.st_mode = S_IFREG | 0666;
     st.st_nlink = 1;
     st.st_atime = info.atime;
     st.st_mtime = info.mtime;
     st.st_ctime = info.ctime;
     st.st_size = info.size;
     printf("   getattr file -> %llu\n", info.size);
   } else {
     yfs_client::dirinfo info;
     ret = yfs->getdir(inum, info);
     if(ret != yfs_client::OK)
       return ret;
     st.st_mode = S_IFDIR | 0777;
     st.st_nlink = 2;
     st.st_atime = info.atime;
     st.st_mtime = info.mtime;
     st.st_ctime = info.ctime;
     printf("   getattr dir -> %lu %lu %lu\n", info.atime, info.mtime, info.ctime);
   }
   return yfs_client::OK;
}


void
fuseserver_getattr(fuse_req_t req, fuse_ino_t ino,
          struct fuse_file_info *fi)
{
    struct stat st;
    yfs_client::inum inum = ino; // req->in.h.nodeid;
    yfs_client::status ret;

    ret = getattr(inum, st);
    if(ret != yfs_client::OK){
      fuse_reply_err(req, ENOENT);
      return;
    }
    fuse_reply_attr(req, &st, 0);
}

/**
  A alteração dos atributos é feita apenas para o atributo 'size' (como enunciado especifica, os outros são facultativos).
  Essencialmente foram criados duas novas funções no 'yfs_client': file_content() e modify_file().
  A primeira função permite aceder ao conteúdo da string associado a um inum no extent server, que seja um ficheiro.
  A segunda função faz "overwrite" da string que está no extent server associada a um inum (que seja ficheiro também) com outra passada por parâmetro.
  A alteração do atributo 'size' é tratada a nível do extent server, que vê qual é o comprimento da string passada por parâmetro e actualiza automaticamente.
  O que esta função setattr() faz, é simplesmente aceder à string no extent server, encurta-a ou aumenta-a (dado o 'size') e volta a guardar no extent server.
*/
void
fuseserver_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi)
{
  printf("fuseserver_setattr 0x%x\n", to_set);
  if (FUSE_SET_ATTR_SIZE & to_set) {
    printf("   fuseserver_setattr set size to %zu\n", attr->st_size);/*

    struct stat st;   //não usar isto ou o 'fuse_file_info' para nada faz algum mal?
    st.st_size = attr->st_size;
    
    //<david>
    //se o ficheiro for maior que o novo tamanho, encurtar a entrada 'extent' no extent server
    //se o ficheiro for menor, inserir padding
    //se o ficheiro for de igual tamanho, não fazer nada
    //a função 'put()' do extent server já lida com alterar automaticamente o tamanho da string em attr

    int size_difference;
    std::string file_data;
    yfs_client::status ret = yfs->file_content(ino, file_data);

    if(ret != yfs_client::OK)
    {

      size_difference = attr->st_size - file_data.size();

      if(size_difference > 0)
        while(size_difference > 0)
        {
          file_data.append(" ");
          size_difference--;
        }
      else if(size_difference < 0)
        file_data = file_data.substr(0, attr->st_size);


      yfs->modify_file(ino, file_data);
      
      fuse_reply_attr(req, fi, 0);
    }
    else
      fuse_reply_err(req, ENOSYS); 
      */
    struct stat st;

    // You fill this in

    if(yfs->isfile(ino))
    {
      yfs->setSize(ino, attr->st_size);
      getattr(ino, st);
      printf("O 'setattr' executa aqui...\n");
      fuse_reply_attr(req, &st, 0);
    }
    else
    {
      printf("O 'setattr' executa aqui se não for ficheiro...\n");
      fuse_reply_err(req, ENOSYS);
    }

  } else {
    printf("O 'setattr' executa aqui se estiver tudo mal...\n");
    fuse_reply_err(req, ENOSYS);
  }
  //<david>

}

void
fuseserver_read(fuse_req_t req, fuse_ino_t ino, size_t size,
      off_t off, struct fuse_file_info *fi)   //nem usei o fuse_file_info, faz mal?
{
  // You fill this in

  //<david>
  if(yfs->isfile(ino))
  {
    std::string file_data;
    yfs->file_content(ino, file_data);

    if(off + size > file_data.size())
    {
      printf("off: %d\nsize: %d\nfile_data_size: %d\n", off, size, file_data.size());
      std::string content = file_data;
      for(int i = file_data.size(); i < off + size; i++)
        content += "\0";
      fuse_reply_buf(req, content.c_str(), strlen(content.c_str()));
    }
    else if(off + size < file_data.size())
    {
      file_data = file_data.substr(off, size);
      fuse_reply_buf(req, file_data.c_str(), strlen(file_data.c_str()));
    }
  }
  else
    fuse_reply_err(req, ENOSYS);
}

void
fuseserver_write(fuse_req_t req, fuse_ino_t ino,
  const char *buf, size_t size, off_t off,
  struct fuse_file_info *fi)
{

  // You fill this in

  //<david>

  printf("\n\n-------------------------------------\n\n");
  printf("\n\nFUSE WRITE function *buf parameter:\n\n%s\n\n", buf);

  if(yfs->isfile(ino))
  {
    std::string file_data;
    yfs->file_content(ino, file_data);

    printf("File content res: %s\n", file_data.c_str());
    printf("off: %d\nsize: %d\nfile_data_size: %d\n", off, size, file_data.size());

    if((off + size) > (file_data.size())) //append
    {

    	printf("APPEND\n\n");

      std::string new_content;
      yfs->file_content(ino, new_content);
      new_content = new_content.substr(0, off);
      new_content += std::string(buf).substr(0, size);
      yfs->modify_file(ino, new_content);

      printf("\n\n-------------------------------------\n\n");
      fuse_reply_write(req, strlen(buf));
    }
    else
    {

    	if((off == 0) && (size < file_data.size())) { //when content is erased
    		printf("CONTENT ERASED\n\n");

    		std::string writen(buf);
		    file_data.replace(off, size, writen);
		    file_data = file_data.substr(0, size);
		    yfs->modify_file(ino, file_data);

		    printf("\n\n-------------------------------------\n\n");
		    fuse_reply_write(req, strlen(buf));
    	} else {									//write in middle of file
    		printf("MIDDLE\n\n");

    		  std::string old_content;
    		  std::string new_content;

		      yfs->file_content(ino, old_content);
		      new_content = old_content.substr(0, off);
		      new_content += std::string(buf).substr(0, size);

		      yfs->file_content(ino, old_content);
		      int len = old_content.size();

		      new_content += old_content.substr(off + size, len);
		      yfs->modify_file(ino, new_content);

		      printf("\n\n-------------------------------------\n\n");
		      fuse_reply_write(req, strlen(buf));

//    		std::string writen(buf);
//		    /*luis*/file_data.replace(off, size, writen);
//		    /*david*///file_data.replace(off, size, writen, off + 1, size);
//		    yfs->modify_file(ino, file_data);

//		    printf("\n\n-------------------------------------\n\n");
//		    fuse_reply_write(req, strlen(buf));
    	}

//      std::string writen(buf);
//      file_data.replace(off, size, writen);
//      file_data = file_data.substr(0, size);
//      yfs->modify_file(ino, file_data);

//      fuse_reply_write(req, strlen(buf));
    }
  }
  else
    fuse_reply_err(req, ENOSYS);

}

yfs_client::status
fuseserver_createhelper(fuse_ino_t parent, const char *name,
     mode_t mode, struct fuse_entry_param *e)
{

  printf("fuseserver_createhelper\n");

  int randnum = rand();
  randnum = randnum & 0x00000000FFFFFFFF;
  randnum = randnum | 0x80000000;
  printf("RANDNUM %lld\n",randnum);

  yfs_client::status ret = yfs->newfile(parent,randnum,name);

	e->ino = randnum;			//inum
	e->generation = 1;	  		//unique number
	e->attr_timeout = 0.0;	    //self-explanatory
	e->entry_timeout = 0.0;	    //self-explanatory

	time_t now;
	time(&now);

	getattr(randnum, e->attr);

	return ret;

}

void
fuseserver_create(fuse_req_t req, fuse_ino_t parent, const char *name,
   mode_t mode, struct fuse_file_info *fi)
{
  struct fuse_entry_param e;
  if( fuseserver_createhelper( parent, name, mode, &e ) == yfs_client::OK ) {
    fuse_reply_create(req, &e, fi);
  } else {
    fuse_reply_err(req, ENOENT);
  }
}

void fuseserver_mknod( fuse_req_t req, fuse_ino_t parent, 
    const char *name, mode_t mode, dev_t rdev ) {
  struct fuse_entry_param e;
  if( fuseserver_createhelper( parent, name, mode, &e ) == yfs_client::OK ) {
    fuse_reply_entry(req, &e);
  } else {
    fuse_reply_err(req, ENOENT);
  }
}

void
fuseserver_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
  struct fuse_entry_param e;
  bool found = false;

  e.attr_timeout = 0.0;
  e.entry_timeout = 0.0;

  printf("fuseserver_lookup\n");

  yfs_client::status ret;

  ret = yfs->ilookup(parent, name);
  if(ret != -1){
  	found = true;
  	e.ino = ret;
  	e.generation = 1;

    struct stat st;
    getattr(ret, st);

    e.attr = st;
  }

  if (found)
    fuse_reply_entry(req, &e);
  else
    fuse_reply_err(req, ENOENT);
}

struct dirbuf {
    char *p;
    size_t size;
};

void dirbuf_add(struct dirbuf *b, const char *name, fuse_ino_t ino)
{
    struct stat stbuf;
    size_t oldsize = b->size;
    b->size += fuse_dirent_size(strlen(name));
    b->p = (char *) realloc(b->p, b->size);
    memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = ino;
    fuse_add_dirent(b->p + oldsize, name, &stbuf, b->size);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
          off_t off, size_t maxsize)
{
  if (off < bufsize)
    return fuse_reply_buf(req, buf + off, min(bufsize - off, maxsize));
  else
    return fuse_reply_buf(req, NULL, 0);
}

void
fuseserver_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
          off_t off, struct fuse_file_info *fi)
{
  yfs_client::inum inum = ino; // req->in.h.nodeid;
  struct dirbuf b;

  printf("fuseserver_readdir\n");

 if(!yfs->isdir(inum)){
    fuse_reply_err(req, ENOTDIR);
    return;
  }

  memset(&b, 0, sizeof(b));

    std::map<yfs_client::inum, std::string> map = yfs->listdir(inum);

    for (std::map<yfs_client::inum, std::string>::iterator it = map.begin(); it!=map.end(); it++) {
      std::string name = it->second;
      dirbuf_add(&b, name.c_str(), it->first);
    }

   reply_buf_limited(req, b.p, b.size, off, size);
   free(b.p);
 }


void
fuseserver_open(fuse_req_t req, fuse_ino_t ino,
     struct fuse_file_info *fi)
{
  // You fill this in

  if(!yfs->isfile(ino))
    fuse_reply_err(req, ENOSYS);
  fuse_reply_open(req, fi);
}

void
fuseserver_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name,
     mode_t mode)
{
  struct fuse_entry_param e;

  // You fill this in
#if 0
  fuse_reply_entry(req, &e);
#else
  fuse_reply_err(req, ENOSYS);
#endif
}

void
fuseserver_unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{

  // You fill this in
  // Success:	fuse_reply_err(req, 0);
  // Not found:	fuse_reply_err(req, ENOENT);
  fuse_reply_err(req, ENOSYS);
}

void
fuseserver_statfs(fuse_req_t req)
{
  struct statvfs buf;

  printf("statfs\n");

  memset(&buf, 0, sizeof(buf));

  buf.f_namemax = 255;
  buf.f_bsize = 512;

  fuse_reply_statfs(req, &buf);
}

struct fuse_lowlevel_ops fuseserver_oper;

int
main(int argc, char *argv[])
{
  char *mountpoint = 0;
  int err = -1;
  int fd;

  setvbuf(stdout, NULL, _IONBF, 0);

  if(argc != 4){
    fprintf(stderr, "Usage: yfs_client <mountpoint> <port-extent-server> <port-lock-server>\n");
    exit(1);
  }
  mountpoint = argv[1];

  srandom(getpid());

  myid = random();

  yfs = new yfs_client(argv[2], argv[3]);

  fuseserver_oper.getattr    = fuseserver_getattr;
  fuseserver_oper.statfs     = fuseserver_statfs;
  fuseserver_oper.readdir    = fuseserver_readdir;
  fuseserver_oper.lookup     = fuseserver_lookup;
  fuseserver_oper.create     = fuseserver_create;
  fuseserver_oper.mknod      = fuseserver_mknod;
  fuseserver_oper.open       = fuseserver_open;
  fuseserver_oper.read       = fuseserver_read;
  fuseserver_oper.write      = fuseserver_write;
  fuseserver_oper.setattr    = fuseserver_setattr;
  fuseserver_oper.unlink     = fuseserver_unlink;
  fuseserver_oper.mkdir      = fuseserver_mkdir;

  const char *fuse_argv[20];
  int fuse_argc = 0;
  fuse_argv[fuse_argc++] = argv[0];
#ifdef __APPLE__
  fuse_argv[fuse_argc++] = "-o";
  fuse_argv[fuse_argc++] = "nolocalcaches"; // no dir entry caching
  fuse_argv[fuse_argc++] = "-o";
  fuse_argv[fuse_argc++] = "daemon_timeout=86400";
#endif

  // everyone can play, why not?
  //fuse_argv[fuse_argc++] = "-o";
  //fuse_argv[fuse_argc++] = "allow_other";

  fuse_argv[fuse_argc++] = mountpoint;
  fuse_argv[fuse_argc++] = "-d";

  fuse_args args = FUSE_ARGS_INIT( fuse_argc, (char **) fuse_argv );
  int foreground;
  int res = fuse_parse_cmdline( &args, &mountpoint, 0 /*multithreaded*/, 
        &foreground );
  if( res == -1 ) {
    fprintf(stderr, "fuse_parse_cmdline failed\n");
    return 0;
  }
  
  args.allocated = 0;

  fd = fuse_mount(mountpoint, &args);
  if(fd == -1){
    fprintf(stderr, "fuse_mount failed\n");
    exit(1);
  }

  struct fuse_session *se;

  se = fuse_lowlevel_new(&args, &fuseserver_oper, sizeof(fuseserver_oper),
       NULL);
  if(se == 0){
    fprintf(stderr, "fuse_lowlevel_new failed\n");
    exit(1);
  }

  struct fuse_chan *ch = fuse_kern_chan_new(fd);
  if (ch == NULL) {
    fprintf(stderr, "fuse_kern_chan_new failed\n");
    exit(1);
  }

  fuse_session_add_chan(se, ch);
  // err = fuse_session_loop_mt(se);   // FK: wheelfs does this; why?
  err = fuse_session_loop(se);
    
  fuse_session_destroy(se);
  close(fd);
  fuse_unmount(mountpoint);

  return err ? 1 : 0;
}
