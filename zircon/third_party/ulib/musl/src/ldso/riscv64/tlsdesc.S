// size_t __tlsdesc_static(size_t *a)
// {
// 	return a[1];
// }
.global __tlsdesc_static
.hidden __tlsdesc_static
.type __tlsdesc_static,@function
__tlsdesc_static:
.hidden __tls_get_new

// size_t __tlsdesc_dynamic(size_t *a)
// {
// 	struct {size_t modidx,off;} *p = (void*)a[1];
// 	size_t *dtv = *(size_t**)(tp + 16 - 8);
// 	if (p->modidx <= dtv[0])
// 		return dtv[p->modidx] + p->off - tp;
// 	return __tls_get_new(p) - tp;
// }
.global __tlsdesc_dynamic
.hidden __tlsdesc_dynamic
.type __tlsdesc_dynamic,@function
__tlsdesc_dynamic:
