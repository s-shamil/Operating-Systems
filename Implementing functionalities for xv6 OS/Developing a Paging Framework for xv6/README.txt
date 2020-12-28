To apply the patch do the following:
cd xv6
cat paging.patch | patch -p1


STEPS TO UNPACK A PATCH TO YOUR REPOSITORY:
git apply <patch file name>
make and make qemu to check
Create a clone of xv6 in your lab machine and apply the patch.
