/*
  Copyright (c) 2020 Sogou, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Authors: Wu Jiaxu (wujiaxu@sogou-inc.com)
*/

#include "WFMySQLServer.h"

#ifdef _WIN32
#include <io.h>
#endif

CommConnection *WFMySQLServer::new_connection(int accept_fd)
{
	CommConnection *conn = this->WFServer::new_connection(accept_fd);

	if (conn)
	{
		protocol::MySQLHandshakeResponse resp;
		struct iovec vec[8];
		int count;

		resp.server_set(0x0a, "5.5", 1, (const uint8_t *)"12345678",
						33, 0, (const uint8_t *)"123456789abc");
		count = resp.encode(vec, 8);
		if (count >= 0)
		{
#ifdef _WIN32
			for (int i = 0; i < count; i++)
				_write(accept_fd, vec[i].iov_base, (unsigned int)vec[i].iov_len);

			return conn;
#else
			if (writev(accept_fd, vec, count) >= 0)
				return conn;
#endif
		}

		delete conn;
	}

	return NULL;
}

CommSession *WFMySQLServer::new_session(long long seq, CommConnection *conn)
{
	static mysql_process_t empty = [](WFMySQLTask *){ };
	WFMySQLTask *task;

	task = WFServerTaskFactory::create_mysql_task(seq ? this->process : empty);
	task->set_keep_alive(this->params.keep_alive_timeout);
	task->set_receive_timeout(this->params.receive_timeout);
	task->get_req()->set_size_limit(this->params.request_size_limit);

	return task;
}
