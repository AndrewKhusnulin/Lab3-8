#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
using namespace boost::asio;
io_service service; //  ������ ������, ������� ������������� ��������� ����� � ��������� ��������� �����/������

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())  //  ��� ������� ����������� � ���������� �������
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class talk_to_svr : public boost::enable_shared_from_this<talk_to_svr> , boost::noncopyable
{
	typedef talk_to_svr self_type;
	talk_to_svr(const std::string & message)
		: sock_(service), started_(true), message_(message) {}
	void start(ip::tcp::endpoint ep)
	{
		sock_.async_connect(ep, MEM_FN1(on_connect, _1)); // ��� ������� ���������� ������������ �� ������� ������
	}
public:
	typedef boost::system::error_code error_code;
	typedef boost::shared_ptr<talk_to_svr> ptr;

	static ptr start(ip::tcp::endpoint ep, const std::string & message)
	{
		ptr new_(new talk_to_svr(message));
		new_->start(ep);
		return new_;
	}
	void stop()
	{
		if (!started_) return;
		started_ = false;
		sock_.close();
	}
	bool started() { return started_; }
private:
	void on_connect(const error_code & err)
	{
		if (!err)
			do_write(message_ + "\'");
		else
			stop();
	}
	void on_read(const error_code & err, size_t bytes)
	{
		if (!err)
		{
			using std::cout;
			using std::endl;
			SetConsoleCP(1251);
			SetConsoleOutputCP(1251);
			std::string copy(read_buffer_, bytes - 1); // �������� ��������� �� �������
			SetConsoleCP(65001);
			SetConsoleOutputCP(65001);
			if (copy == message_) // ���� ����������� ������ ������ (���������  �� ������� == �������� �������)
			{
				SetConsoleCP(65001);
				SetConsoleOutputCP(65001);
				cout << endl << "Thanks, ";
				SetConsoleOutputCP(1251);
				cout << copy << "!" << endl;
			}
			else
			{
				if (copy.find("admin admin") != -1) // ���� ����������� �����, ��� ���� ������, ��� ��������� �� ������� != ���������, � ����� ������ ��������, ���� �� ������� �����
				{
					SetConsoleOutputCP(65001);
					cout << endl << "server clients:" << endl;
					SetConsoleOutputCP(1251);
					cout << copy << endl;
				}
				else
				{
					SetConsoleOutputCP(65001);
					cout << endl << "[-] Error. You are disconnected from the server." << endl;
				}
			}

		}
		else
		{
			std::cout << std::endl << "[-] Error " << err << " You are disconnected from the server." << std::endl;
		}
		stop();
	}

	void on_write(const error_code & err, size_t bytes)
	{
		do_read();
	}
	void do_read()
	{
		async_read(sock_, buffer(read_buffer_), MEM_FN2(read_complete, _1, _2), MEM_FN2(on_read, _1, _2)); // ��� ������� ���������� ������
																										   //�� ������. �� ���������� ���������� ����������.
	}
	void do_write(const std::string & msg)
	{
		if (!started()) return;
		std::copy(msg.begin(), msg.end(), write_buffer_);
		sock_.async_write_some(buffer(write_buffer_, msg.size()), MEM_FN2(on_write, _1, _2)); // ��� ������� ��������� �������� ����������� �������� ������ �� ������
	}
	size_t read_complete(const boost::system::error_code & err, size_t bytes)
	{
		if (err) return 0;
		bool found = std::find(read_buffer_, read_buffer_ + bytes, '\'') < read_buffer_ + bytes;
		// ���������������� ������, ���� �� ���������� ����� ������, ��� �����������
		return found ? 0 : 1;
	}

private:
	ip::tcp::socket sock_; // ����� - ����� �����  �������� � ��������
	enum { max_msg = 10240 };
	char read_buffer_[max_msg]; // ����� ��� ������
	char write_buffer_[max_msg]; // ����� ������
	bool started_;
	std::string message_; // ���������� ��������� �������
};

int main()
{
	// ����������� ��������
	SetConsoleOutputCP(65001);
	std::cout << std::endl << "*** Lab �8: Client ***" << std::endl;
	std::string message;
	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001); // �������� �����, ��������� - ����� � ����
	std::cout << std::endl << "Enter your name: ";
	std::string name;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	getline(std::cin, name);
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	std::cout << std::endl << "Enter your surname: ";
	std::string surname;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	getline(std::cin, surname);
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	unsigned int pos = { 0 };
	while ((pos = name.find(" ")) != -1)
	{
		name.erase(pos);
	}
	message = name + " " + surname;
	if (message.size() > 10230)
	{
		std::cout << std::endl << "[-]the message is too long";
		system("pause");
		return 0;
	}
	talk_to_svr::start(ep, message);
	boost::this_thread::sleep(boost::posix_time::millisec(100));
	service.run(); // �������� ������� (�� ���� ��� ������, ������������ �������� ������/������ � ����� ���� �� ���� ����� �������� �������� ������������ ��)
	system("pause");
	return 0;
}
