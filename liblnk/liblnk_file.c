/*
 * File functions
 *
 * Copyright (c) 2009-2010, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>
#include <libnotify.h>

#include "liblnk_codepage.h"
#include "liblnk_data_string.h"
#include "liblnk_debug.h"
#include "liblnk_definitions.h"
#include "liblnk_io_handle.h"
#include "liblnk_file.h"
#include "liblnk_file_information.h"
#include "liblnk_libbfio.h"
#include "liblnk_location_information.h"
#include "liblnk_shell_item_identifiers_list.h"

/* Initialize a file
 * Make sure the value file is pointing to is set to NULL
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_initialize(
     liblnk_file_t **file,
     liberror_error_t **error )
{
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_initialize";

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	if( *file == NULL )
	{
		internal_file = (liblnk_internal_file_t *) memory_allocate(
		                                            sizeof( liblnk_internal_file_t ) );

		if( internal_file == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create file.",
			 function );

			return( -1 );
		}
		if( memory_set(
		     internal_file,
		     0,
		     sizeof( liblnk_internal_file_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear file.",
			 function );

			memory_free(
			 internal_file );

			return( -1 );
		}
		if( liblnk_file_information_initialize(
		     &( internal_file->file_information ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create file information.",
			 function );

			memory_free(
			 internal_file );

			return( -1 );
		}
		if( liblnk_io_handle_initialize(
		     &( internal_file->io_handle ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create IO handle.",
			 function );

			liblnk_file_information_free(
			 &( internal_file->file_information ),
			 NULL );
			memory_free(
			 internal_file );

			return( -1 );
		}
		*file = (liblnk_file_t *) internal_file;
	}
	return( 1 );
}

/* Frees a file
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_free(
     liblnk_file_t **file,
     liberror_error_t **error )
{
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_free";
	int result                            = 1;

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	if( *file != NULL )
	{
		internal_file = (liblnk_internal_file_t *) *file;

		if( internal_file->file_io_handle != NULL )
		{
			if( liblnk_file_close(
			     *file,
			     error ) != 0 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_CLOSE_FAILED,
				 "%s: unable to close file.",
				 function );

				result = -1;
			}
		}
		*file = NULL;

		if( liblnk_io_handle_free(
		     &( internal_file->io_handle ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free IO handle.",
			 function );

			result = -1;
		}
		memory_free(
		 internal_file );
	}
	return( result );
}

/* Signals the file to abort its current activity
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_signal_abort(
     liblnk_file_t *file,
     liberror_error_t **error )
{
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_signal_abort";

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	internal_file = (liblnk_internal_file_t *) file;

	if( internal_file->io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal file - missing IO handle.",
		 function );

		return( -1 );
	}
	internal_file->io_handle->abort = 1;

	return( 1 );
}

/* Opens a file
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_open(
     liblnk_file_t *file,
     const char *filename,
     int access_flags,
     liberror_error_t **error )
{
	libbfio_handle_t *file_io_handle      = NULL;
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_open";

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	internal_file = (liblnk_internal_file_t *) file;

	if( filename == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBLNK_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBLNK_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported accesss flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBLNK_ACCESS_FLAG_WRITE ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access to Windows Shortcut files currently not supported.",
		 function );

		return( -1 );
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libbfio_handle_set_track_offsets_read(
	     file_io_handle,
	     1,
	     error ) != 1 )
	{
                liberror_error_set(
                 error,
                 LIBERROR_ERROR_DOMAIN_RUNTIME,
                 LIBERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set track offsets read in file IO handle.",
                 function );

		libbfio_handle_free(
		 &file_io_handle,
		 NULL );

                return( -1 );
	}
#endif
	if( libbfio_file_set_name(
	     file_io_handle,
	     filename,
	     libcstring_narrow_string_length(
	      filename ) + 1,
	     error ) != 1 )
	{
                liberror_error_set(
                 error,
                 LIBERROR_ERROR_DOMAIN_RUNTIME,
                 LIBERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set filename in file IO handle.",
                 function );

		libbfio_handle_free(
		 &file_io_handle,
		 NULL );

                return( -1 );
	}
	if( liblnk_file_open_file_io_handle(
	     file,
	     file_io_handle,
	     access_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open file: %s.",
		 function,
		 filename );

		libbfio_handle_free(
		 &file_io_handle,
		 NULL );

		return( -1 );
	}
	internal_file->file_io_handle_created_in_library = 1;

	return( 1 );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE )

/* Opens a file
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_open_wide(
     liblnk_file_t *file,
     const wchar_t *filename,
     int access_flags,
     liberror_error_t **error )
{
	libbfio_handle_t *file_io_handle      = NULL;
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_open_wide";

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	internal_file = (liblnk_internal_file_t *) file;

	if( filename == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBLNK_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBLNK_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBLNK_ACCESS_FLAG_WRITE ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access to Windows Shortcut files currently not supported.",
		 function );

		return( -1 );
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libbfio_handle_set_track_offsets_read(
	     file_io_handle,
	     1,
	     error ) != 1 )
	{
                liberror_error_set(
                 error,
                 LIBERROR_ERROR_DOMAIN_RUNTIME,
                 LIBERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set track offsets read in file IO handle.",
                 function );

		libbfio_handle_free(
		 &file_io_handle,
		 NULL );

                return( -1 );
	}
#endif
	if( libbfio_file_set_name_wide(
	     file_io_handle,
	     filename,
	     libcstring_wide_string_length(
	      filename ) + 1,
	     error ) != 1 )
	{
                liberror_error_set(
                 error,
                 LIBERROR_ERROR_DOMAIN_RUNTIME,
                 LIBERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set filename in file IO handle.",
                 function );

		libbfio_handle_free(
		 &file_io_handle,
		 NULL );

                return( -1 );
	}
	if( liblnk_file_open_file_io_handle(
	     file,
	     file_io_handle,
	     access_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open file: %ls.",
		 function,
		 filename );

		libbfio_handle_free(
		 &file_io_handle,
		 NULL );

		return( -1 );
	}
	internal_file->file_io_handle_created_in_library = 1;

	return( 1 );
}

#endif

/* Opens a file using a Basic File IO (bfio) handle
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_open_file_io_handle(
     liblnk_file_t *file,
     libbfio_handle_t *file_io_handle,
     int access_flags,
     liberror_error_t **error )
{
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_open_file_io_handle";
	int bfio_access_flags                 = 0;
	int file_io_handle_is_open            = 0;

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	internal_file = (liblnk_internal_file_t *) file;

	if( internal_file->file_io_handle != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid internal file - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( file_io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file IO handle.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBLNK_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBLNK_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBLNK_ACCESS_FLAG_WRITE ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access to  Windows Shortcut files currently not supported.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBLNK_ACCESS_FLAG_READ ) != 0 )
	{
		bfio_access_flags = LIBBFIO_FLAG_READ;
	}
	internal_file->file_io_handle = file_io_handle;

	file_io_handle_is_open = libbfio_handle_is_open(
	                          internal_file->file_io_handle,
	                          error );

	if( file_io_handle_is_open == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open file.",
		 function );

		return( -1 );
	}
	else if( file_io_handle_is_open == 0 )
	{
		if( libbfio_handle_open(
		     internal_file->file_io_handle,
		     bfio_access_flags,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_OPEN_FAILED,
			 "%s: unable to open file IO handle.",
			 function );

			return( -1 );
		}
	}
	if( liblnk_file_open_read(
	     internal_file,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read from file handle.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Closes a file
 * Returns 0 if successful or -1 on error
 */
int liblnk_file_close(
     liblnk_file_t *file,
     liberror_error_t **error )
{
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_close";
	int result                            = 0;

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	internal_file = (liblnk_internal_file_t *) file;

	if( internal_file->file_io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file - missing file IO handle.",
		 function );

		return( -1 );
	}
	if( internal_file->file_io_handle_created_in_library != 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			if( liblnk_debug_print_read_offsets(
			     internal_file->file_io_handle,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print the read offsets.",
				 function );

				result = -1;
			}
		}
#endif
		if( libbfio_handle_close(
		     internal_file->file_io_handle,
		     error ) != 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_CLOSE_FAILED,
			 "%s: unable to close file IO handle.",
			 function );

			result = -1;
		}
		if( libbfio_handle_free(
		     &( internal_file->file_io_handle ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file IO handle.",
			 function );

			result = -1;
		}
	}
	internal_file->file_io_handle                    = NULL;
	internal_file->file_io_handle_created_in_library = 0;

	if( internal_file->file_information != NULL )
	{
		if( liblnk_file_information_free(
		     &( internal_file->file_information ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file information.",
			 function );

			result = -1;
		}
	}
	if( internal_file->link_target_identifier != NULL )
	{
		if( liblnk_shell_item_identifiers_list_free(
		     &( internal_file->link_target_identifier ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free link target identifier.",
			 function );

			result = -1;
		}
	}
	if( internal_file->location_information != NULL )
	{
		if( liblnk_location_information_free(
		     &( internal_file->location_information ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free location information.",
			 function );

			result = -1;
		}
	}
	if( internal_file->description != NULL )
	{
		if( liblnk_data_string_free(
		     &( internal_file->description ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free description.",
			 function );

			result = -1;
		}
	}
	if( internal_file->relative_path != NULL )
	{
		if( liblnk_data_string_free(
		     &( internal_file->relative_path ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free relative path.",
			 function );

			result = -1;
		}
	}
	if( internal_file->working_directory != NULL )
	{
		if( liblnk_data_string_free(
		     &( internal_file->working_directory ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free working directory.",
			 function );

			result = -1;
		}
	}
	if( internal_file->command_line_arguments != NULL )
	{
		if( liblnk_data_string_free(
		     &( internal_file->command_line_arguments ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free command line arguments.",
			 function );

			result = -1;
		}
	}
	if( internal_file->icon_location != NULL )
	{
		if( liblnk_data_string_free(
		     &( internal_file->icon_location ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free icon location.",
			 function );

			result = -1;
		}
	}
	return( result );
}

/* Opens a file for reading
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_open_read(
     liblnk_internal_file_t *internal_file,
     liberror_error_t **error )
{
	static char *function     = "liblnk_file_open_read";
	off64_t file_offset       = 0;
	ssize_t read_count        = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint8_t *trailing_data    = NULL;
	size_t trailing_data_size = 0;
#endif

	if( internal_file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal file.",
		 function );

		return( -1 );
	}
	if( internal_file->io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal file - missing IO handle.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "Reading file header:\n" );
	}
#endif
	read_count = liblnk_io_handle_read_file_header(
	              internal_file->io_handle,
	              internal_file->file_io_handle,
	              internal_file->class_identifier,
	              16,
	              internal_file->file_information,
	              error );

	if( read_count <= 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read file header.",
		 function );

		return( -1 );
	}
	file_offset += read_count;

	if( ( internal_file->io_handle->data_flags & LIBLNK_DATA_FLAG_HAS_LINK_TARGET_IDENTIFIER ) != 0 )
	{
		if( liblnk_shell_item_identifiers_list_initialize(
		     &( internal_file->link_target_identifier ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create link target identifier.",
			 function );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading shell items:\n" );
		}
#endif
		read_count = liblnk_shell_item_identifiers_list_read(
		              internal_file->link_target_identifier,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read link target identifier.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
	if( ( internal_file->io_handle->data_flags & LIBLNK_DATA_FLAG_HAS_LOCATION_INFORMATION ) != 0 )
	{
		if( liblnk_location_information_initialize(
		     &( internal_file->location_information ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create location information.",
			 function );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading location information:\n" );
		}
#endif
		read_count = liblnk_location_information_read(
		              internal_file->location_information,
		              internal_file->io_handle,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read location information.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
	if( ( internal_file->io_handle->data_flags & LIBLNK_DATA_FLAG_HAS_DESCRIPTION_STRING ) != 0 )
	{
		if( liblnk_data_string_initialize(
		     &( internal_file->description ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create description.",
			 function );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading description data string:\n" );
		}
#endif
		read_count = liblnk_data_string_read(
		              internal_file->description,
		              internal_file->io_handle,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read description.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
	if( ( internal_file->io_handle->data_flags & LIBLNK_DATA_FLAG_HAS_RELATIVE_PATH_STRING ) != 0 )
	{
		if( liblnk_data_string_initialize(
		     &( internal_file->relative_path ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create relative path.",
			 function );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading relative path data string:\n" );
		}
#endif
		read_count = liblnk_data_string_read(
		              internal_file->relative_path,
		              internal_file->io_handle,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read relative path.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
	if( ( internal_file->io_handle->data_flags & LIBLNK_DATA_FLAG_HAS_WORKING_DIRECTORY_STRING ) != 0 )
	{
		if( liblnk_data_string_initialize(
		     &( internal_file->working_directory ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create working directory.",
			 function );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading working directory data string:\n" );
		}
#endif
		read_count = liblnk_data_string_read(
		              internal_file->working_directory,
		              internal_file->io_handle,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read working directory.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
	if( ( internal_file->io_handle->data_flags & LIBLNK_DATA_FLAG_HAS_COMMAND_LINE_ARGUMENTS_STRING ) != 0 )
	{
		if( liblnk_data_string_initialize(
		     &( internal_file->command_line_arguments ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create command line arguments.",
			 function );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading command line arguments data string:\n" );
		}
#endif
		read_count = liblnk_data_string_read(
		              internal_file->command_line_arguments,
		              internal_file->io_handle,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read command line arguments.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
	if( ( internal_file->io_handle->data_flags & LIBLNK_DATA_FLAG_HAS_ICON_LOCATION_STRING ) != 0 )
	{
		if( liblnk_data_string_initialize(
		     &( internal_file->icon_location ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create icon location.",
			 function );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading icon location data string:\n" );
		}
#endif
		read_count = liblnk_data_string_read(
		              internal_file->icon_location,
		              internal_file->io_handle,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read icon location.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
	if( file_offset < (off64_t) internal_file->io_handle->file_size )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "Reading extra data blocks:\n" );
		}
#endif
		read_count = liblnk_io_handle_read_data_blocks(
		              internal_file->io_handle,
		              internal_file->file_io_handle,
		              file_offset,
		              error );

		if( read_count <= -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read extra data blocks.",
			 function );

			return( -1 );
		}
		file_offset += read_count;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( file_offset < (off64_t) internal_file->io_handle->file_size )
		{
			trailing_data_size = (size_t) ( internal_file->io_handle->file_size - file_offset );

			trailing_data = (uint8_t *) memory_allocate(
						     sizeof( uint8_t ) * trailing_data_size );

			if( trailing_data == NULL )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_MEMORY,
				 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
				 "%s: unable to create trailing data.",
				 function );

				return( -1 );
			}
			read_count = libbfio_handle_read(
				      internal_file->file_io_handle,
				      trailing_data,
				      trailing_data_size,
				      error );

			if( read_count != (ssize_t) trailing_data_size )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read trailing data.",
				 function );

				memory_free(
				 trailing_data );

				return( -1 );
			}
			file_offset += read_count;

			libnotify_printf(
			 "%s: trailing data:\n",
			 function );
			libnotify_print_data(
			 trailing_data,
			 trailing_data_size );

			memory_free(
			 trailing_data );
		}
	}
#endif
	return( 1 );
}

/* Retrieves the file ASCII codepage
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_get_ascii_codepage(
     liblnk_file_t *file,
     int *ascii_codepage,
     liberror_error_t **error )
{
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_get_ascii_codepage";

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	internal_file = (liblnk_internal_file_t *) file;

	if( internal_file->io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal file - missing IO handle.",
		 function );

		return( -1 );
	}
	if( ascii_codepage == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid ASCII codepage.",
		 function );

		return( -1 );
	}
	*ascii_codepage = internal_file->io_handle->ascii_codepage;

	return( 1 );
}

/* Sets the file ASCII codepage
 * Returns 1 if successful or -1 on error
 */
int liblnk_file_set_ascii_codepage(
     liblnk_file_t *file,
     int ascii_codepage,
     liberror_error_t **error )
{
	liblnk_internal_file_t *internal_file = NULL;
	static char *function                 = "liblnk_file_set_ascii_codepage";

	if( file == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file.",
		 function );

		return( -1 );
	}
	internal_file = (liblnk_internal_file_t *) file;

	if( internal_file->io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal file - missing IO handle.",
		 function );

		return( -1 );
	}
	if( ( ascii_codepage != LIBLNK_CODEPAGE_ASCII )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_874 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1250 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1251 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1252 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1253 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1254 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1256 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1257 )
	 && ( ascii_codepage != LIBLNK_CODEPAGE_WINDOWS_1258 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported ASCII codepage.",
		 function );

		return( -1 );
	}
	internal_file->io_handle->ascii_codepage = ascii_codepage;

	return( 1 );
}

