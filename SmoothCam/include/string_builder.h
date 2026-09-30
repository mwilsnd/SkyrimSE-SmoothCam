#pragma once

template<typename T>
struct StringBuilder {
	StringBuilder() = default;
	~StringBuilder() {};

	void append(T&& v) noexcept {
		size_ += v.length();
		items.push_back(std::move(v));
	}

	void clear() noexcept {
		items.clear();
		size_ = 0;
	}

	void clearAndShrink() noexcept {
		clear();
		items.shrink_to_fit();
	}

	size_t size() const noexcept {
		return items.size();
	}

	T build() noexcept {
		str.clear();
		str.reserve(size_);
		for (const auto& s : items)
			str.append(s);
		return str;
	}

	T& get() noexcept {
		str.clear();
		str.reserve(size_);
		for (const auto& s : items)
			str.append(s);
		return str;
	}

	private:
		T str = {};
		std::vector<T> items = {};
		size_t size_ = 0;
};