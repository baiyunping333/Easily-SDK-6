
#include <xdl.h>

#ifdef _OS_WINDOWS
#include <conio.h>
#endif


//#define URL		_T("http://localhost:8889/loc/mymovi.mp4")
//#define URL		_T("https://localhost:8888/loc/mymovi.mp4")
//#define URL		_T("https://myssl.com:443/www.sspanda.com?status=q")
//#define URL		_T("https://www.baidu.com:443")
//#define URL		_T("https://mp.weixin.qq.com:443")
#define URL		_T("https:/139.196.196.107:443")

void set_ssl(xhand_t ssl)
{
	file_t pxf = { 0 };
    byte_t buf_crt[X509_CERT_SIZE] = { 0 };
    byte_t buf_rsa[RSA_KEY_SIZE] = { 0 };
	dword_t dw_crt = 0;
	dword_t dw_key = 0;
    
    pxf = xfile_open(NULL, _T("../sbin/ssl/sslsrv.crt"), 0);
    if (pxf)
    {
        dw_crt = X509_CERT_SIZE;
        xfile_read(pxf, buf_crt, dw_crt);
		dw_crt = a_xslen((schar_t*)buf_crt);
       
        xfile_close(pxf);
    }
    
    pxf = xfile_open(NULL, _T("../sbin/ssl/sslsrv.key"), 0);
    if (pxf)
    {
        dw_key = RSA_KEY_SIZE;
        xfile_read(pxf, buf_rsa, dw_key);
		dw_key = a_xslen((schar_t*)buf_rsa);

        xfile_close(pxf);
    }

	xssl_set_cert(ssl, buf_crt, dw_crt);
	xssl_set_rsa(ssl, buf_rsa, dw_key, _T("123456"), -1);
}

void test_ssl_srv()
{
    net_addr_t sin;
    res_file_t so;
    sys_info_t si = { 0 };
    
    so = socket_tcp(0, FILE_OPEN_OVERLAP);
    if (so == INVALID_FILE)
    {
        return;
    }
    
    xmem_zero((void*)&sin, sizeof(sin));
    
    fill_addr(&sin, 8888, NULL);
    
    if (!socket_bind(so, (res_addr_t)&sin, sizeof(sin)))
    {
        socket_close(so);
        return; //bind sock error
    }
    
    if (!socket_listen(so, SOMAXCONN))
    {
        socket_close(so);
        return; //listen error
    }
    
    net_addr_t locaddr, rmtaddr;
    int addr_len;
    res_file_t ao;
	async_t over = { 0 };
	async_init(&over, ASYNC_EVENT, TCP_BASE_TIMO, INVALID_FILE);
    
    addr_len = sizeof(net_addr_t);
    ao = socket_accept(so, (res_addr_t)&rmtaddr, &addr_len, &over);
    if (ao == INVALID_FILE)
    {
        async_uninit(&over);
        socket_close(so);
        return;
    }
    
    xhand_t ssl = xssl_srv(ao);
    if(!ssl)
    {
		async_uninit(&over);
        socket_close(ao);
        socket_close(so);
        return;
    }
    
    set_ssl(ssl);
    
    byte_t buf[100] = {0};
    dword_t dw = 100;
    xssl_read(ssl, buf, &dw);
    
    printf("%s", (char*)buf);
    
    xssl_close(ssl);
    
    socket_close(ao);
    socket_close(so);

	async_uninit(&over);
}

void test_ssl_cli()
{
	tchar_t addr[ADDR_LEN + 1] = { 0 };

	//host_addr(_T("mp.weixin.qq.com"), addr);
	//host_addr(_T("www.baidu.com"), addr);
	xscpy(addr, _T("127.0.0.1"));

    xhand_t ssl = xssl_cli(443, addr);
    
    if(!ssl)
        return;
    
	xssl_set_version(ssl, SSLv30);

	set_ssl(ssl);

    byte_t buf[10] = {'0','1','2','3','4','5','6','7','8','9'};
    dword_t dw = 100;

    xssl_write(ssl, buf, &dw);
    
    xssl_close(ssl);
}

void test_dtls_cli()
{
	tchar_t addr[ADDR_LEN + 1] = { 0 };

	//host_addr(_T("mp.weixin.qq.com"), addr);
	//host_addr(_T("www.baidu.com"), addr);
	xscpy(addr, _T("127.0.0.1"));

	xhand_t ssl = xdtls_cli(443, addr);

	if (!ssl)
		return;

	xdtls_set_version(ssl, DTLSv0);

	set_ssl(ssl);

	byte_t buf[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	dword_t dw = 100;

	xdtls_write(ssl, buf, &dw);

	xdtls_close(ssl);
}

int main()
{
    xdk_process_init(XDK_APARTMENT_PROCESS);
    
    //test_ssl_srv();
    
    test_ssl_cli();

	//test_dtls_cli();
    
    xdk_process_uninit();
    
    return 0;
}

