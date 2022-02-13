
#include <xdk.h>

#ifdef _OS_WINDOWS
#include <conio.h>
#endif

#define URL_GET			_T("tftps://127.0.0.1:69/demo.txt")
//#define URL_GET			_T("tftp://118.178.180.81:69/demo.txt")
//#define URL_GET			_T("tftp://49.234.135.113:69/demo.txt")
//#define URL_GET			_T("tftp://172.16.190.190:69/demo.txt")
//#define URL_GET			_T("tftp://172.16.190.200:69/demo.txt")

#define DATA_SIZE (32 * 1024 * 1024)

void test_tftp_put()
{
	xhand_t tftp = xtftp_client(_T("PUT"), URL_GET);

	if (tftp)
	{
		xdtls_set_version(xtftp_bio(tftp), DTLSv2);

		if (xtftp_connect(tftp))
		{
			byte_t* data = (byte_t*)xmem_alloc(DATA_SIZE);
			dword_t dw = 0;

			while (dw < DATA_SIZE)
			{
				data[dw] = dw % 256;
				dw++;
			}

			if (xtftp_send(tftp, data, &dw))
			{
				xtftp_flush(tftp);
			}

			xmem_free(data);
		}

		xtftp_close(tftp);
	}
}

void test_tftp_get()
{
	xhand_t tftp = xtftp_client(_T("GET"), URL_GET);
	
	if (tftp)
	{
		xdtls_set_version(xtftp_bio(tftp), DTLSv2);

		if (xtftp_connect(tftp))
		{
			byte_t* data = (byte_t*)xmem_alloc(DATA_SIZE);
			dword_t dw = DATA_SIZE;

			xtftp_recv(tftp, data, &dw);

			if (!dw)
			{
				_tprintf(_T("read failed!\n"));
			}

			while (dw--)
			{
				if (data[dw] != dw % 256)
					_tprintf(_T("%d not passed!\n"), dw);
			}

			xmem_free(data);
		}

		xtftp_close(tftp);
	}

	_tprintf(_T("END!\n"));
}

void test_tftp_head()
{
	xhand_t tftp = xtftp_client(_T("HEAD"), URL_GET);

	xtftp_connect(tftp);

	xtftp_head(tftp);

	xtftp_close(tftp);

}

void test_tftp_del()
{
	xhand_t tftp = xtftp_client(_T("DELETE"), URL_GET);

	xtftp_connect(tftp);

	xtftp_delete(tftp);

	xtftp_close(tftp);
}

int _main(int argc, char* argv[])
{
	tchar_t errtext[ERR_LEN + 1] = { 0 };
    
	xdk_process_init(XDK_APARTMENT_THREAD | XDK_INITIALIZE_CONSOLE);
    
	test_tftp_put();

	//test_tftp_get();

	//test_tftp_head();

	//test_tftp_del();

	xdk_process_uninit();

	printf("%s\n", errtext);

#ifdef _OS_WINDOWS

	getch();
#endif

	return 0;
}

