// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2016, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_HTTP_LOW_LEVEL_SESSION_HPP_
#define POSEIDON_HTTP_LOW_LEVEL_SESSION_HPP_

#include "../mutex.hpp"
#include "../tcp_session_base.hpp"
#include "server_reader.hpp"
#include "server_writer.hpp"
#include "request_headers.hpp"
#include "response_headers.hpp"
#include "status_codes.hpp"

namespace Poseidon {

namespace Http {
	class UpgradedSessionBase;
	class HeaderOption;

	class LowLevelSession : public TcpSessionBase, private ServerReader, private ServerWriter {
		friend UpgradedSessionBase;

	private:
		mutable Mutex m_upgraded_session_mutex;
		boost::shared_ptr<UpgradedSessionBase> m_upgraded_session;

	public:
		explicit LowLevelSession(UniqueFile socket);
		~LowLevelSession();

	protected:
		const boost::shared_ptr<UpgradedSessionBase> &get_low_level_upgraded_session() const {
			// Epoll 线程读取不需要锁。
			return m_upgraded_session;
		}

		// TcpSessionBase
		void on_read_hup() NOEXCEPT OVERRIDE;
		void on_close(int err_code) NOEXCEPT OVERRIDE;

		void on_read_avail(StreamBuffer data) OVERRIDE;

		// ServerReader
		void on_request_headers(RequestHeaders request_headers, boost::uint64_t content_length) OVERRIDE;
		void on_request_entity(boost::uint64_t entity_offset, StreamBuffer entity) OVERRIDE;
		bool on_request_end(boost::uint64_t content_length, OptionalMap headers) OVERRIDE;

		// ServerWriter
		long on_encoded_data_avail(StreamBuffer encoded) OVERRIDE;

		// 可覆写。
		virtual void on_low_level_request_headers(RequestHeaders request_headers, boost::uint64_t content_length) = 0;
		virtual void on_low_level_request_entity(boost::uint64_t entity_offset, StreamBuffer entity) = 0;
		virtual boost::shared_ptr<UpgradedSessionBase> on_low_level_request_end(boost::uint64_t content_length, OptionalMap headers) = 0;

	public:
		boost::shared_ptr<UpgradedSessionBase> get_upgraded_session() const;

		bool send(ResponseHeaders response_headers, StreamBuffer entity = StreamBuffer());
		bool send(StatusCode status_code);
		bool send(StatusCode status_code, StreamBuffer entity, const HeaderOption &content_type);
		bool send(StatusCode status_code, OptionalMap headers, StreamBuffer entity = StreamBuffer());
		bool send_default(StatusCode status_code, OptionalMap headers = OptionalMap());

		bool send_chunked_header(ResponseHeaders response_headers);
		bool send_chunk(StreamBuffer entity);
		bool send_chunked_trailer(OptionalMap headers);
	};
}

}

#endif
