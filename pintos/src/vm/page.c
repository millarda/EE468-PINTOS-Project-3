#include "vm/page.h"
#include "threads/pte.h"
#include "vm/frame.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "string.h"
#include "userprog/syscall.h"
#include "vm/swap.h"

void vm_page_init ();
struct sup_page_entry * get_spe(struct hash *ht, void * user_vaddr);
bool load_page(struct sup_page_entry *spe);
bool load_file_page(struct sup_page_entry *spe);

void vm_page_init (){
  return;
}

//find hashelement corresponding to a certain user virtual address in a hash table. return success
struct sup_page_entry * get_spe(struct hash *h, void * user_vaddr){
  struct sup_page_entry spe;
  struct hash_elem *he;

  spe.user_vaddr = user_vaddr;
  he = hash_find(h, &spe.elem);
  if(he != NULL){
    return hash_entry(he, struct sup_page_entry, elem);
  }
  return NULL
}

//load page NOTE this may need to be changed to account for mmf and swapping
bool load_page(struct sup_page_entry *spe){
  if(spe->type == FILE){
    load_file_page(spe);
  }
}

//load a page that is type file. return success
bool load_file_page(struct sup_page_entry *spe){
  struct thread *curr = thread_current ();

  file_seek (spe->data.file, spe->data.offset);//look in file at offset

  uint8_t *page = vm_get_frame(PAL_USER);//allocate a page
  if(page == NULL){//allocation must have failed, load unsuccessful
    return false;
  }

  if(file_read(spe->data.file, page, spe->data.read_bytes) != (int) spe->data.read_bytes){//check if file is at certain offset. clear frame and return fail if it is
    vm_free_frame(page);
    return false;
  }

  memset (page + spe->data.read_bytes, 0, spe->data.zero_bytes);//set the zero bytes of the page to 0

  if(!pagedir_set_page(curr->pagedir, spe->user_vaddr, page, spe->data.writable)){//attempt to map page to physical
    vm_free_frame(page);//if failed to map, then free frame and return unsuccessful
    return false;
  }

//load was succesful, indicate as such
  spe->loaded = true;
  return true;


}
