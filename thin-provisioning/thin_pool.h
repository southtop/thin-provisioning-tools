// Copyright (C) 2011 Red Hat, Inc. All rights reserved.
//
// This file is part of the thin-provisioning-tools source.
//
// thin-provisioning-tools is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// thin-provisioning-tools is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with thin-provisioning-tools.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef MULTISNAP_METADATA_H
#define MULTISNAP_METADATA_H

#include "metadata.h"

#include <string>

//----------------------------------------------------------------

namespace thin_provisioning {
	// This interface is very like that in the kernel.  It'll allow us
	// to write simulators to try out different space maps etc.  Not
	// currently used by the tools.

	class thin_pool;
	class thin {
	public:
		struct lookup_result {
			block_address block_;
			bool shared_;
		};

		typedef std::shared_ptr<thin> ptr;
		typedef boost::optional<lookup_result> maybe_address;

		thin_dev_t get_dev_t() const;
		maybe_address lookup(block_address thin_block);
		bool insert(block_address thin_block, block_address data_block);
		void remove(block_address thin_block);

		void set_snapshot_time(uint32_t time);

		block_address get_mapped_blocks() const;
		void set_mapped_blocks(block_address count);

	private:
		friend class thin_pool;
		thin(thin_dev_t dev, thin_pool &pool);
		thin(thin_dev_t dev, thin_pool &pool,
		     device_tree_detail::device_details const &details);

		thin_dev_t dev_;
		thin_pool &pool_;
		device_tree_detail::device_details details_;
		uint32_t open_count_;
		bool changed_;
	};

	class thin_pool {
	public:
		typedef std::shared_ptr<thin_pool> ptr;

		thin_pool(block_manager::ptr bm);
		thin_pool(block_manager::ptr bm,
			  sector_t data_block_size,
			  block_address nr_data_blocks);
		~thin_pool();

		void create_thin(thin_dev_t dev);
		void create_snap(thin_dev_t dev, thin_dev_t origin);
		void del(thin_dev_t);
		void commit();

		void set_transaction_id(uint64_t id);
		uint64_t get_transaction_id() const;

		// handling metadata snapshot
		void reserve_metadata_snap();
		void release_metadata_snap();
		block_address get_metadata_snap() const;

		block_address alloc_data_block();
		void free_data_block(block_address b);

		// accessors
		block_address get_nr_free_data_blocks() const;
		sector_t get_data_block_size() const;
		block_address get_data_dev_size() const;
		uint32_t get_time() const;

		thin::ptr open_thin(thin_dev_t);
		void close_thin(thin::ptr td);

		// updates the superblock
		void set_needs_check();

	private:
		friend class thin;
		typedef std::map<thin_dev_t, thin::ptr> device_map;

		bool device_exists(thin_dev_t dev) const;
		thin::ptr create_device(thin_dev_t dev);
		thin::ptr open_device(thin_dev_t dev);
		void close_device(thin::ptr device);
		void set_snapshot_details(thin::ptr snap, thin_dev_t origin);
		void write_changed_details();

		metadata::ptr md_;
		device_map thin_devices_;
	};

	void process_read(thin::ptr td, thin_pool::ptr tp, sector_t offset);
	void process_write(thin::ptr td, thin_pool::ptr tp, sector_t offset);
	void process_discard(thin::ptr td, thin_pool::ptr tp, sector_t offset);
};

//----------------------------------------------------------------

#endif
