#pragma once
#include <stdexcept>
#include <chrono>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <pulse/simple.h>
#include <pulse/timeval.h>
#include <pulse/error.h>

#include <pulse/mainloop.h>
#include <pulse/thread-mainloop.h>
#include <pulse/context.h>

namespace pulseaudio {
	class exception : public std::runtime_error {
		int m_code;
	public:
		exception(const std::string& msg, int error = PA_ERR_UNKNOWN)
			: std::runtime_error(msg), m_code{error}
		{}
		exception(int error)
			: exception(pa_strerror(error), error)
		{}

		int error_code() const noexcept { return m_code; }
	};

	class simple_client {
	protected:
		pa_simple* m_con = nullptr;
		pa_sample_spec m_spec{};
		pa_buffer_attr m_buf_attr{};

		simple_client(pa_stream_direction_t dir, const char* name, uint32_t sample_rate = 16000, uint32_t channels = 1) {
			connect(dir, name, sample_rate, channels);
		}

		void connect(pa_stream_direction_t dir, const char* name, uint32_t sample_rate = 16000, uint32_t channels = 1) {
			if (m_con) pa_simple_free(m_con);
			m_con = nullptr;
			m_spec.channels = channels;
			m_spec.rate = sample_rate;
			m_spec.format = PA_SAMPLE_S16NE;
			m_buf_attr.maxlength = -1;
			m_buf_attr.minreq = -1;
			m_buf_attr.prebuf = -1;
			// Aim for a latency of about 100ms
			m_buf_attr.tlength = pa_usec_to_bytes(100 * PA_USEC_PER_MSEC, &m_spec);
			m_buf_attr.fragsize = m_buf_attr.tlength;
			int error;
			m_con = pa_simple_new(NULL, "snowboy", dir, NULL, name, &m_spec, NULL, &m_buf_attr, &error);
			if (m_con == nullptr)
				throw exception(error);
		}

	public:

		~simple_client() {
			if (m_con) pa_simple_free(m_con);
		}

		simple_client(const simple_client&) = delete;
		simple_client& operator=(const simple_client&) = delete;
		simple_client(simple_client&& other)
			: m_con(other.m_con), m_spec(other.m_spec), m_buf_attr(other.m_buf_attr) {
			other.m_con = nullptr;
			other.m_spec = {};
			other.m_buf_attr = {};
		}
		simple_client& operator=(simple_client&& other) {
			std::swap(m_con, other.m_con);
			std::swap(m_spec, other.m_spec);
			std::swap(m_buf_attr, other.m_buf_attr);
			return *this;
		}

		void flush() {
			int error;
			if (pa_simple_flush(m_con, &error) < 0)
				throw exception(error);
		}

		std::chrono::microseconds get_latency() const {
			int error = -1;
			auto res = pa_simple_get_latency(m_con, &error);
			if(error != -1)
				throw exception(error);

			return std::chrono::microseconds{res};
		}
	};

	class simple_record_stream : public simple_client {
	public:
		simple_record_stream(const char* name, uint32_t sample_rate = 16000, uint32_t channels = 1)
			: simple_client(PA_STREAM_RECORD, name, sample_rate, channels)
		{}

		void connect(const char* name, uint32_t sample_rate = 16000, uint32_t channels = 1) {
			simple_client::connect(PA_STREAM_RECORD, name, sample_rate, channels);
		}

		void read(std::vector<short>& samples) {
			if (samples.empty()) samples.resize(m_spec.rate / 10);
			int error;
			if (pa_simple_read(m_con, samples.data(), samples.size() * sizeof(short), &error) < 0)
				throw exception(error);
		}
	};

	class simple_playback_stream : public simple_client {
	public:
		simple_playback_stream(const char* name, uint32_t sample_rate = 16000, uint32_t channels = 1)
			: simple_client(PA_STREAM_PLAYBACK, name, sample_rate, channels)
		{}

		void connect(const char* name, uint32_t sample_rate = 16000, uint32_t channels = 1) {
			simple_client::connect(PA_STREAM_PLAYBACK, name, sample_rate, channels);
		}

		void write(const std::vector<short>& samples) {
			int error;
			if (pa_simple_write(m_con, samples.data(), samples.size() * sizeof(short), &error) < 0)
				throw exception(error);
		}

		void drain() {
			int error;
			if (pa_simple_drain(m_con, &error) < 0)
				throw exception(error);
		}
	};

	class disable_copy_and_move {
	protected:
		disable_copy_and_move() = default;
		disable_copy_and_move(const disable_copy_and_move&) = delete;
		disable_copy_and_move(disable_copy_and_move&&) = delete;
		disable_copy_and_move& operator=(const disable_copy_and_move&) = delete;
		disable_copy_and_move& operator=(disable_copy_and_move&&) = delete;
	};

	class mainloop : public disable_copy_and_move {
		pa_mainloop* m_loop;
		std::function<int(struct pollfd *ufds, unsigned long nfds, int timeout)> m_poll_fn;
	public:
		mainloop() {
			m_loop = pa_mainloop_new();
			if(m_loop == nullptr)
				throw exception("failed to create mainloop object");
		}

		~mainloop() noexcept {
			if(m_loop) pa_mainloop_free(m_loop);
		}

		int dispatch() {
			auto res = pa_mainloop_dispatch(m_loop);
			if(res < 0)
				throw exception(res);
			return res;
		}

		pa_mainloop_api* get_api() {
			auto res = pa_mainloop_get_api(m_loop);
			if(res == nullptr)
				throw exception("failed to get api from mainloop");
			return res;
		}

		int get_retval() const noexcept {
			return pa_mainloop_get_retval(m_loop);
		}

		int iterate(bool block = true) {
			int retval;
			auto res = pa_mainloop_iterate(m_loop, block ? 1 : 0, &retval);
			if(res < 0)
				throw exception(res);
			return retval;
		}

		void poll() {
			auto res = pa_mainloop_poll(m_loop);
			if(res < 0)
				throw exception(res);
		}

		bool prepare(std::chrono::microseconds timeout = std::chrono::microseconds(-1)) {
			auto res = pa_mainloop_prepare(m_loop, timeout.count());
			return res >= 0;
		}

		void quit(int retval = 0) noexcept {
			pa_mainloop_quit(m_loop, retval);
		}

		int run() {
			int retval;
			auto res = pa_mainloop_run(m_loop, &retval);
			if(res < 0)
				throw exception(res);
			return retval;
		}

		void set_poll_func(pa_poll_func cb, void* userdata) {
			pa_mainloop_set_poll_func(m_loop, cb, userdata);
		}

		void set_poll_func(std::function<int(struct pollfd *ufds, unsigned long nfds, int timeout)> fn) {
			m_poll_fn = fn;
			set_poll_func([](struct pollfd *ufds, unsigned long nfds, int timeout, void* userdata)->int{
				return static_cast<mainloop*>(userdata)->m_poll_fn(ufds, nfds, timeout);
			}, this);
		}

		void wakeup() {
			pa_mainloop_wakeup(m_loop);
		}
	};

	class threaded_mainloop : public disable_copy_and_move {
		pa_threaded_mainloop* m_loop;
	public:
		threaded_mainloop() {
			m_loop = pa_threaded_mainloop_new();
			if(m_loop == nullptr)
				throw exception("failed to create mainloop object");
		}

		~threaded_mainloop() noexcept {
			if(m_loop) pa_threaded_mainloop_free(m_loop);
		}

		void accept() {
			pa_threaded_mainloop_accept(m_loop);
		}

		bool in_thread() {
			return pa_threaded_mainloop_in_thread(m_loop) != 0;
		}

		void lock() {
			pa_threaded_mainloop_lock(m_loop);
		}

		#if PA_MAJOR >= 13
		void once_unlocked(void(*cb)(pa_threaded_mainloop*, void*), void* userdata) {
			pa_threaded_mainloop_once_unlocked(m_loop, cb, userdata);
		}
		#endif

		void set_name(const char* name) {
			#if PA_MAJOR >= 5
			pa_threaded_mainloop_set_name(m_loop, name);
			#endif
		}

		void signal(bool wait) {
			pa_threaded_mainloop_signal(m_loop, wait ? 1 : 0);
		}

		void start() {
			auto res = pa_threaded_mainloop_start(m_loop);
			if(res < 0)
				throw exception(res);
		}

		void stop() {
			pa_threaded_mainloop_stop(m_loop);
		}

		void unlock() {
			pa_threaded_mainloop_unlock(m_loop);
		}

		void wait() {
			pa_threaded_mainloop_wait(m_loop);
		}

		pa_mainloop_api* get_api() {
			auto res = pa_threaded_mainloop_get_api(m_loop);
			if(res == nullptr)
				throw exception("failed to get api from mainloop");
			return res;
		}

		int get_retval() const noexcept {
			return pa_threaded_mainloop_get_retval(m_loop);
		}
	};

	class operation {
		pa_operation* m_op = nullptr;
	public:
		operation(pa_operation* op, bool ref = true) {
			if(ref) {
				if(op) m_op = pa_operation_ref(op);
				else m_op = op;
			}
		}
		~operation() noexcept {
			if(m_op) pa_operation_unref(m_op);
		}

		void cancel() {
			if(!valid()) throw exception("empty operation");
			pa_operation_cancel(m_op);
		}

		pa_operation_state_t get_state() const {
			if(!valid()) throw exception("empty operation");
			return pa_operation_get_state(m_op);
		}

		void set_state_callback(pa_operation_notify_cb_t cb, void* userdata) {
			if(!valid()) throw exception("empty operation");
			pa_operation_set_state_callback(m_op, cb, userdata);
		}

		bool running() const { return get_state() == PA_OPERATION_RUNNING; }
		bool done() const { return get_state() == PA_OPERATION_DONE; }
		bool cancelled() const { return get_state() == PA_OPERATION_CANCELLED; }

		void wait() {
			struct info {
				std::mutex mtx;
				std::condition_variable cv;
			} i;
			std::unique_lock<std::mutex> lck{i.mtx};
			if(!running()) return;
			set_state_callback([](pa_operation *o, void *userdata){
				auto x = static_cast<struct info*>(userdata);
				std::unique_lock<std::mutex> lck{x->mtx};
				x->cv.notify_all();
			}, &i);
			while(running()) {
				i.cv.wait(lck);
			}
			set_state_callback(nullptr, nullptr);
		}

		bool valid() const noexcept { return m_op != nullptr; }
		operator bool() const noexcept { return valid(); }
		bool operator!() const noexcept { return !valid(); }
	};

	class context {
		pa_context* m_ctx;
	public:
		context(pa_context* c, bool ref = true) {
			if(ref) m_ctx = pa_context_ref(c);
			else m_ctx = c;
		}
		context(pa_mainloop_api* ml, const char* name) {
			m_ctx = pa_context_new(ml, name);
		}
		context(mainloop& ml, const char* name)
			: context(ml.get_api(), name)
		{}
		context(threaded_mainloop& ml, const char* name)
			: context(ml.get_api(), name)
		{}
		~context() {
			if(m_ctx) pa_context_unref(m_ctx);
		}
		context(const context& other) {
			m_ctx = pa_context_ref(other.m_ctx);
		}
		context& operator=(const context& other) {
			if(m_ctx) pa_context_unref(m_ctx);
			m_ctx = pa_context_ref(other.m_ctx);
			return *this;
		}
		context(context&&) = delete;
		context& operator=(context&&) = delete;

		void connect(const char* server = nullptr, pa_context_flags_t flags = PA_CONTEXT_NOFLAGS, const pa_spawn_api* api = nullptr) {
			auto res = pa_context_connect(m_ctx, server, flags, api);
			if(res < 0) throw exception(last_errno());
		}

		void disconnect() {
			pa_context_disconnect(m_ctx);
		}

		operation drain(pa_context_notify_cb_t cb = nullptr, void* userdata = nullptr) {
			return operation{pa_context_drain(m_ctx, cb, userdata), false};
		}

		int last_errno() const noexcept {
			return pa_context_errno(m_ctx);
		}

		operation exit_daemon(pa_context_success_cb_t cb = nullptr, void* userdata = nullptr) {
			return operation{pa_context_exit_daemon(m_ctx, cb, userdata), false};
		}

		#if PA_CHECK_VERSION(0,9,11)
		uint32_t get_index() const {
			auto res = pa_context_get_index(m_ctx);
			if(res == PA_INVALID_INDEX)
				throw exception(last_errno());
			return res;
		}
		#endif

		uint32_t get_protocol_version() const {
			return pa_context_get_protocol_version(m_ctx);
		}

		const char* get_server() const {
			return pa_context_get_server(m_ctx);
		}

		uint32_t get_server_protocol_version() const {
			auto res = pa_context_get_server_protocol_version(m_ctx);
			if(res == PA_INVALID_INDEX)
				throw exception(last_errno());
			return res;
		}

		pa_context_state_t get_state() const {
			return pa_context_get_state(m_ctx);
		}

		size_t get_tile_size(const pa_sample_spec* ss = nullptr) const {
			return pa_context_get_tile_size(m_ctx, ss);
		}

		bool is_local() const {
			return pa_context_is_local(m_ctx) == 1;
		}

		bool is_pending() const {
			return pa_context_is_pending(m_ctx) != 0;
		}

		void load_cookie_from_file(const char* file) {
			if(pa_context_load_cookie_from_file(m_ctx, file) < 0)
				throw exception(last_errno());
		}

		#if PA_CHECK_VERSION(0,9,11)
		operation proplist_remove(const char* const * keys, pa_context_success_cb_t cb = nullptr, void* userdata = nullptr) {
			return operation{pa_context_proplist_remove(m_ctx, keys, cb, userdata), false};
		}

		operation proplist_update(pa_update_mode_t mode, const pa_proplist* p, pa_context_success_cb_t cb = nullptr, void* userdata = nullptr) {
			return operation{pa_context_proplist_update(m_ctx, mode, const_cast<pa_proplist*>(p), cb, userdata), false};
		}

		// TODO: Rework this
		pa_time_event* rttime_new(pa_usec_t usec, pa_time_event_cb_t cb, void* userdata) {
			return pa_context_rttime_new(m_ctx, usec, cb, userdata);
		}

		// TODO: Rework this
		void rttime_new(pa_time_event* e, pa_usec_t usec) {
			pa_context_rttime_restart(m_ctx, e, usec);
		}
		#endif

		operation set_default_sink(const char* name, pa_context_success_cb_t cb = nullptr, void* userdata = nullptr) {
			return operation{pa_context_set_default_sink(m_ctx, name, cb, userdata), false};
		}

		operation set_default_source(const char* name, pa_context_success_cb_t cb = nullptr, void* userdata = nullptr) {
			return operation{pa_context_set_default_source(m_ctx, name, cb, userdata), false};
		}

		void set_event_callback(pa_context_event_cb_t cb, void* userdata) {
			pa_context_set_event_callback(m_ctx, cb, userdata);
		}

		operation set_name(const char* name, pa_context_success_cb_t cb = nullptr, void* userdata = nullptr) {
			return operation{pa_context_set_name(m_ctx, name, cb, userdata), false};
		}

		void set_state_callback(pa_context_notify_cb_t cb, void* userdata) {
			pa_context_set_state_callback(m_ctx, cb, userdata);
		}
	};


} // namespace pulseaudio
