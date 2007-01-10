
#pragma once

#include "SvnEnums.h"
#include "SvnCommitArgs.h"

namespace SharpSvn {
	using System::Collections::Generic::ICollection;
	using System::Collections::Generic::IList;

	ref class SvnException;

	public ref class SvnClientEventArgs abstract : public System::EventArgs
	{
		apr_pool_t *_pool;

	internal:
		SvnClientEventArgs(apr_pool_t *pool)
		{
			_pool = pool;
		}

	protected:
		SvnClientEventArgs()
		{
		}

	public:
		/// <summary>Detaches the SvnClientEventArgs from the unmanaged storage; optionally keeping the property values for later use</summary>
		/// <description>After this method is called all properties are either stored managed, or are no longer readable</description>
		virtual void Detach(bool keepProperties)
		{
			_pool = nullptr;
		}
	};

	public ref class SvnClientCancelEventArgs : public SvnClientEventArgs
	{
		bool _cancel;

	public:
		SvnClientCancelEventArgs()
		{
		}

		property bool Cancel
		{
			bool get()
			{
				return _cancel;
			}
			void set(bool value)
			{
				_cancel = value;
			}
		}
	};

	public ref class SvnClientProgressEventArgs : public SvnClientEventArgs
	{
		initonly __int64 _progress;
		initonly __int64 _totalProgress;


	internal:
		SvnClientProgressEventArgs(__int64 progress, __int64 totalProgress, apr_pool_t *pool)
			: SvnClientEventArgs(pool)
		{
			_progress = progress;
			_totalProgress = totalProgress;
		}

	public:
		SvnClientProgressEventArgs(__int64 progress, __int64 totalProgress)
		{
			_progress = progress;
			_totalProgress = totalProgress;
		}

	public:
		property __int64 Progress
		{
			__int64 get()
			{
				return _progress;
			}
		}

		property __int64 TotalProgress
		{
			__int64 get()
			{
				return _totalProgress;
			}
		}
	};

	public ref class SvnClientBeforeCommitEventArgs : public SvnClientEventArgs
	{
		AprPool^ _pool;
		const apr_array_header_t *_commitItems;
		bool _cancel;
		String^ _logMessage;
		IList<SvnCommitItem^>^ _items;
	internal:
		SvnClientBeforeCommitEventArgs(const apr_array_header_t *commitItems, AprPool^ pool);

	public:
		property bool Cancel
		{
			bool get()
			{
				return _cancel;
			}
			void set(bool value)
			{
				_cancel = value;
			}
		}

		property String^ LogMessage
		{
			String^ get()
			{
				return _logMessage;
			}
			void set(String^ value)
			{
				_logMessage = value;
			}
		}

		property IList<SvnCommitItem^>^ Items
		{
			IList<SvnCommitItem^>^ get();
		}

		virtual void Detach(bool keepProperties) override;
	};

	public ref class SvnLockInfo : public SvnClientEventArgs
	{
		const svn_lock_t *_lock;
		String^ _path;
		String^ _fullPath;
		String^ _token;
		String^ _owner;
		String^ _comment;
		initonly bool _isDavComment;
		initonly DateTime _creationDate;
		initonly DateTime _expirationDate;
	internal:
		SvnLockInfo(const svn_lock_t *lock)
		{
			if(!lock)
				throw gcnew ArgumentNullException("lock");

			_lock = lock;
			_isDavComment = lock->is_dav_comment != 0;
			_creationDate = SvnBase::DateTimeFromAprTime(lock->creation_date);
			_expirationDate = SvnBase::DateTimeFromAprTime(lock->expiration_date);
		}

	public:
		property String^ Path
		{
			String^ get()
			{
				if(!_path && _lock)
					_path = SvnBase::Utf8_PtrToString(_lock->path);

				return _path;
			}
		}

		/// <summary>The path the notification is about, translated via <see cref="System::IO::Path::GetFullPath" /></summary>
		/// <remarks>The <see cref="FullPath" /> property contains the path in normalized format; while <see cref="Path" /> returns the exact path from the subversion api</remarks>
		property String^ FullPath
		{
			String^ get()
			{
				if(!_fullPath && Path)
					_fullPath = System::IO::Path::GetFullPath(Path);

				return _fullPath;
			}
		}

		property String^ Token
		{
			String^ get()
			{
				if(!_token && _lock)
					_token = SvnBase::Utf8_PtrToString(_lock->token);

				return _token;
			}
		}

		property String^ Owner
		{
			String^ get()
			{
				if(!_owner && _lock)
					_owner = SvnBase::Utf8_PtrToString(_lock->owner);

				return _owner;
			}
		}

		property String^ Comment
		{
			String^ get()
			{
				if(!_comment && _lock)
					_comment = SvnBase::Utf8_PtrToString(_lock->comment);

				return _comment;
			}
		}

		property bool IsWebDavComment
		{
			bool get()
			{
				return _isDavComment;
			}
		}

		property DateTime CreationDate
		{
			DateTime get()
			{
				return _creationDate;
			}
		}

		property DateTime ExpirationDate
		{
			DateTime get()
			{
				return _expirationDate;
			}
		}



	public:
		virtual void Detach(bool keepProperties) override
		{
			try
			{
				if(keepProperties)
				{
					GC::KeepAlive(Path);
					GC::KeepAlive(Token);
					GC::KeepAlive(Owner);
					GC::KeepAlive(Comment);
				}
			}
			finally
			{
				_lock = nullptr;
				__super::Detach(keepProperties);
			}
		}
	};

	public ref class SvnClientNotifyEventArgs : public SvnClientEventArgs
	{
		const svn_wc_notify_t *_notify;
		initonly SvnNotifyAction _action;
		initonly SvnNodeKind _nodeKind;
		initonly SvnNotifyState _contentState;
		initonly SvnNotifyState _propertyState;
		initonly SvnLockState _lockState;
		initonly __int64 _revision;
		SvnLockInfo^ _lock;
		
	internal:
		SvnClientNotifyEventArgs(const svn_wc_notify_t *notify, apr_pool_t *pool)
			: SvnClientEventArgs(pool)
		{
			if(!notify)
				throw gcnew ArgumentNullException("notify");

			_notify = notify;
			_action = (SvnNotifyAction)notify->action;
			_nodeKind = (SvnNodeKind)notify->kind;
			_contentState = (SvnNotifyState)notify->content_state;
			_propertyState = (SvnNotifyState)notify->prop_state;
			_lockState = (SvnLockState)notify->lock_state;
			_revision = notify->revision;

			// TODO: Add support for the Lock member (containing the lock owner etc.)
		}

	private:
		String^ _fullPath;
		String^ _path;
		String^ _mimeType;
		SvnException^ _exception;
	public:
		/// <summary>The path the notification is about</summary>
		/// <remarks>The <see cref="FullPath" /> property contains the path in normalized format; while <see cref="Path" /> returns the exact path from the subversion api</remarks>
		property String^ Path
		{
			String^ get()
			{
				if(!_path && _notify)
					_path = SvnBase::Utf8_PtrToString(_notify->path);

				return _path;
			}
		}

		/// <summary>The path the notification is about, translated via <see cref="System::IO::Path::GetFullPath" /></summary>
		/// <remarks>The <see cref="FullPath" /> property contains the path in normalized format; while <see cref="Path" /> returns the exact path from the subversion api</remarks>
		property String^ FullPath
		{
			String^ get()
			{
				if(!_fullPath && Path)
					_fullPath = System::IO::Path::GetFullPath(Path);

				return _fullPath;
			}
		}

		property SvnNotifyAction Action
		{
			SvnNotifyAction get()
			{
				return _action;
			}
		}

		property SvnNodeKind NodeKind
		{
			SvnNodeKind get()
			{
				return _nodeKind;
			}
		}

		property String^ MimeType
		{
			String^ get()
			{
				if(!_mimeType && _notify && _notify->mime_type)
					_mimeType = SvnBase::Utf8_PtrToString(_notify->mime_type);

				return _mimeType;
			}
		}

		property SvnException^ Error
		{
			SvnException^ get();
		}

		property SvnNotifyState ContentState
		{
			SvnNotifyState get()
			{
				return _contentState;
			}
		}

		property SvnNotifyState PropertyState
		{
			SvnNotifyState get()
			{
				return _propertyState;
			}
		}

		property SvnLockState LockState
		{
			SvnLockState get()
			{
				return _lockState;
			}
		}

		property __int64 Revision
		{
			__int64 get()
			{
				return _revision;
			}
		}

		property SvnLockInfo^ Lock
		{
			SvnLockInfo^ get()
			{
				if(!_lock && _notify && _notify->lock)
					_lock = gcnew SvnLockInfo(_notify->lock);

				return _lock;
			}
		}


	public:
		/// <summary>Detaches the SvnClientEventArgs from the unmanaged storage; optionally keeping the property values for later use</summary>
		/// <description>After this method is called all properties are either stored managed, or are no longer readable</description>
		virtual void Detach(bool keepProperties) override
		{
			try
			{
				if(keepProperties)
				{
					// Use all properties to get them cached in .Net memory
					GC::KeepAlive(Path);
					GC::KeepAlive(MimeType);
					GC::KeepAlive(Error);
					GC::KeepAlive(Lock);
				}

				if(_lock)
					_lock->Detach(keepProperties);
			}
			finally
			{
				_notify = nullptr;
				__super::Detach(keepProperties);
			}
		}
	};

	public ref class SvnStatusEventArgs : public SvnClientEventArgs
	{
		initonly String^ _path;
		const svn_wc_status2_t *_status;
		String^ _fullPath;
		initonly SvnWcStatus _wcContentStatus;
		initonly SvnWcStatus _wcPropertyStatus;
		initonly bool _wcLocked;
		initonly bool _copied;
		initonly bool _switched;
		initonly SvnWcStatus _reposContentStatus;
		initonly SvnWcStatus _reposPropertyStatus;
		SvnLockInfo^ _reposLock;
		Uri^ _uri;
		initonly __int64 _oodLastCommitRev;
		initonly DateTime _oodLastCommitDate;
		initonly SvnNodeKind _oodLastCommitNodeKind;
		String^ _oodLastCommitAuthor;


	internal:
		SvnStatusEventArgs(String^ path, const svn_wc_status2_t *status)
		{
			if(!path)
				throw gcnew ArgumentNullException("path");
			else if(!status)
				throw gcnew ArgumentNullException("status");

			_path = path;
			_status = status;

			_wcContentStatus = (SvnWcStatus)status->text_status;
			_wcPropertyStatus = (SvnWcStatus)status->prop_status;
			_wcLocked = status->locked != 0;
			_copied = status->copied != 0;
			_switched = status->switched != 0;
			_reposContentStatus = (SvnWcStatus)status->repos_text_status;
			_reposPropertyStatus = (SvnWcStatus)status->repos_prop_status;

			_oodLastCommitRev = status->ood_last_cmt_rev;

			if(status->ood_last_cmt_rev != SVN_INVALID_REVNUM)
			{
				_oodLastCommitDate = SvnBase::DateTimeFromAprTime(status->ood_last_cmt_date);
				_oodLastCommitNodeKind = (SvnNodeKind)status->ood_kind;
			}
		}

	public:
		property String^ Path
		{
			String^ get()
			{
				return _path;
			}
		}

		/// <summary>The path the notification is about, translated via <see cref="System::IO::Path::GetFullPath" /></summary>
		/// <remarks>The <see cref="FullPath" /> property contains the path in normalized format; while <see cref="Path" /> returns the exact path from the subversion api</remarks>
		property String^ FullPath
		{
			String^ get()
			{
				if(!_fullPath && Path)
					_fullPath = System::IO::Path::GetFullPath(Path);

				return _fullPath;
			}
		}

		/// <summary>Content status in working copy</summary>
		property SvnWcStatus WcContentStatus
		{
			SvnWcStatus get()
			{
				return _wcContentStatus;
			}
		}

		/// <summary>Property status in working copy</summary>
		property SvnWcStatus WcPropertyStatus
		{
			SvnWcStatus get()
			{
				return _wcPropertyStatus;
			}
		}
		
		/// <summary>Gets a boolean indicating whether the workingcopy is locked</summary>
		property bool WcLocked
		{
			bool get()
			{
				return _wcLocked;
			}
		}

		/// <summary>Gets a boolean indicating whether the file is copied in the working copy</summary>
		/// <remarks>A file or directory can be 'copied' if it's scheduled for addition-with-history 
		/// (or part of a subtree that is scheduled as such.).</remarks>
		property bool WcCopied
		{
			bool get()
			{
				return _copied;
			}
		}

		/// <summary>Gets a boolean indicating whether the file is switched in the working copy</summary>
		property bool WcSwitched
		{
			bool get()
			{
				return _switched;
			}
		}

		property SvnWcStatus ReposContentStatus
		{
			SvnWcStatus get()
			{
				return _reposContentStatus;
			}
		}

		property SvnWcStatus ReposPropertyStatus
		{
			SvnWcStatus get()
			{
				return _reposPropertyStatus;
			}
		}

		property SvnLockInfo^ ReposLock
		{
			SvnLockInfo^ get()
			{
				if(!_reposLock && _status && _status->repos_lock)
					_reposLock = gcnew SvnLockInfo(_status->repos_lock);

				return _reposLock;
			}
		}

		property Uri^ Uri
		{
			System::Uri^ get()
			{
				if(!_uri && _status && _status->url)
					_uri = gcnew System::Uri(SvnBase::Utf8_PtrToString(_status->url));

				return _uri;
			}
		}

		property bool IsOutOfDate
		{
			bool get()
			{
				return _oodLastCommitRev != SVN_INVALID_REVNUM;
			}
		}

		property __int64 OodLastCommitRevision
		{
			__int64 get()
			{
				return _oodLastCommitRev;
			}
		}

		property DateTime OodLastCommitDate
		{
			DateTime get()
			{
				return _oodLastCommitDate;
			}
		}

		property String^ OodCommitAuthor
		{
			String^ get()
			{
				if(!_oodLastCommitAuthor && _status && _status->ood_last_cmt_author && IsOutOfDate)
					_oodLastCommitAuthor = SvnBase::Utf8_PtrToString(_status->ood_last_cmt_author);

				return _oodLastCommitAuthor;
			}
		}

	public:
		virtual void Detach(bool keepProperties) override
		{
			try
			{
				if(keepProperties)
				{
					// Use all properties to get them cached in .Net memory
					GC::KeepAlive(Uri);
					GC::KeepAlive(ReposLock);
					GC::KeepAlive(OodCommitAuthor);
				}

				if(_reposLock)
					_reposLock->Detach(keepProperties);
			}
			finally
			{
				_status = nullptr;
				__super::Detach(keepProperties);
			}
		}
	};

	public ref class SvnInfoEventArgs : public SvnClientEventArgs
	{
		const svn_info_t* _info;
		initonly String^ _path;
		String^ _fullPath;
		Uri^ _uri;
		initonly __int64 _rev;
		initonly SvnNodeKind _nodeKind;
		Uri^ _reposRootUri;
		String^ _reposUuid;
		initonly __int64 _lastChangeRev;
		initonly DateTime _lastChangeDate;
		String^ _lastChangeAuthor;
		SvnLockInfo^ _lock;
		initonly bool _hasWcInfo;
		SvnWcSchedule _wcSchedule;
		Uri ^_copyFromUri;
		initonly __int64 _copyFromRev;
		initonly DateTime _contentTime;
		initonly DateTime _propertyTime;
		String^ _checksum;
		String^ _conflict_old;
		String^ _conflict_new;
		String^ _conflict_wrk;
		String^ _prejfile;

	internal:
		SvnInfoEventArgs(String^ path, const svn_info_t* info)
		{
			if(!path)
				throw gcnew ArgumentNullException("path");
			else if(!info)
				throw gcnew ArgumentNullException("info");

			_info = info;
			_path = path;
			_rev = info->rev;
			_nodeKind = (SvnNodeKind)info->kind;
			if(info->last_changed_author)
			{
				_lastChangeRev = info->last_changed_rev;
				_lastChangeDate = SvnBase::DateTimeFromAprTime(info->last_changed_date);
			}
			_hasWcInfo = info->has_wc_info != 0;
			_wcSchedule = (SvnWcSchedule)info->schedule;
			_copyFromRev = info->copyfrom_rev;
			if(_hasWcInfo)
			{
				_contentTime = SvnBase::DateTimeFromAprTime(info->text_time);
				_propertyTime = SvnBase::DateTimeFromAprTime(info->prop_time);
			}			
		}
	public:
		property String^ Path
		{
			String^ get()
			{
				return _path;
			}
		}

		/// <summary>The path the notification is about, translated via <see cref="System::IO::Path::GetFullPath" /></summary>
		/// <remarks>The <see cref="FullPath" /> property contains the path in normalized format; while <see cref="Path" /> returns the exact path from the subversion api</remarks>
		property String^ FullPath
		{
			String^ get()
			{
				if(!_fullPath && Path)
					_fullPath = System::IO::Path::GetFullPath(Path);

				return _fullPath;
			}
		}

		property Uri^ Uri
		{
			System::Uri^ get()
			{
				if(!_uri && _info && _info->URL)
					_uri = gcnew System::Uri(SvnBase::Utf8_PtrToString(_info->URL));

				return _uri;
			}
		}

		property __int64 Revision
		{
			__int64 get()
			{
				return _rev;
			}
		}

		property SvnNodeKind NodeKind
		{
			SvnNodeKind get()
			{
				return _nodeKind;
			}
		}

		/// <summary>Gets the repository root Uri; ending in a '/' at the end</summary>
		/// <remarks>The unmanaged api does not add the '/' at the end, but this makes using <see cref="System::Uri" /> hard</remarks>
		property System::Uri^ ReposRoot
		{
			System::Uri^ get()
			{
				if(!_reposRootUri && _info && _info->repos_root_URL)
				{
					// Subversion does not add a / at the end; but to use the Uri infrastructure we require a /
					_reposRootUri = gcnew System::Uri(SvnBase::Utf8_PtrToString(_info->repos_root_URL) + "/");
				}

				return _reposRootUri;
			}
		}
		
		property Guid ReposUuid
		{
			Guid get()
			{
				if(!_reposUuid && _info && _info->repos_UUID)
					_reposUuid = SvnBase::Utf8_PtrToString(_info->repos_UUID);

				return _reposUuid ? Guid(_reposUuid) : Guid::Empty;
			}
		}

		property __int64 LastChangeRevision
		{
			__int64 get()
			{
				return _lastChangeRev;
			}
		}

		property DateTime LastChangeDate
		{
			DateTime get()
			{
				return _lastChangeDate;
			}
		}

		property String^ LastChangeAuthor
		{
			String^ get()
			{
				if(!_lastChangeAuthor && _info && _info->last_changed_author)
					_lastChangeAuthor = SvnBase::Utf8_PtrToString(_info->last_changed_author);

				return _lastChangeAuthor;
			}
		}

		property SvnLockInfo^ Lock
		{
			SvnLockInfo^ get()
			{
				if(!_lock && _info && _info->lock)
					_lock = gcnew SvnLockInfo(_info->lock);

				return _lock;
			}
		}

		property bool HasWcInfo
		{
			bool get()
			{
				return _hasWcInfo;
			}
		}

		property SvnWcSchedule Schedule
		{
			SvnWcSchedule get()
			{
				return HasWcInfo ? _wcSchedule : SvnWcSchedule::None;
			}
		}

		property System::Uri^ CopyFromUri
		{
			System::Uri^ get()
			{
				if(!_copyFromUri && _info && _info->copyfrom_url && HasWcInfo)
					_copyFromUri = gcnew System::Uri(SvnBase::Utf8_PtrToString(_info->copyfrom_url));

				return _copyFromUri;
			}
		}

		property __int64 CopyFromRev
		{
			__int64 get()
			{
				return _copyFromRev;
			}
		}

		property DateTime ContentTime
		{
			DateTime get()
			{
				return _contentTime; 
			}
		}

		property DateTime PropertyTime
		{
			DateTime get()
			{
				return _propertyTime;
			}
		}

		property String^ Checksum
		{
			String^ get()
			{
				if(!_checksum && _info && _info->checksum && HasWcInfo)
					_checksum = SvnBase::Utf8_PtrToString(_info->checksum);

				return _checksum;
			}
		}

		property String^ ConflictOld
		{
			String^ get()
			{
				if(!_conflict_old && _info && _info->conflict_old && HasWcInfo)
					_conflict_old = SvnBase::Utf8_PtrToString(_info->conflict_old);

				return _conflict_old;
			}
		}

		property String^ ConflictNew
		{
			String^ get()
			{
				if(!_conflict_new && _info && _info->conflict_new && HasWcInfo)
					_conflict_new = SvnBase::Utf8_PtrToString(_info->conflict_new);

				return _conflict_new;
			}
		}

		property String^ ConflictWork
		{
			String^ get()
			{
				if(!_conflict_wrk && _info && _info->conflict_wrk && HasWcInfo)
					_conflict_wrk = SvnBase::Utf8_PtrToString(_info->conflict_wrk);

				return _conflict_wrk;
			}
		}

		property String^ PropertyEditFile
		{
			String^ get()
			{
				if(!_prejfile && _info && _info->prejfile && HasWcInfo)
					_prejfile = SvnBase::Utf8_PtrToString(_info->prejfile);

				return _prejfile;
			}
		}

	public:
		virtual void Detach(bool keepProperties) override
		{
			try
			{
				if(keepProperties)
				{
					// Use all properties to get them cached in .Net memory
					GC::KeepAlive(Path);
					GC::KeepAlive(Uri);
					GC::KeepAlive(ReposRoot);
					GC::KeepAlive(ReposUuid);
					GC::KeepAlive(LastChangeAuthor);
					GC::KeepAlive(Lock);
					GC::KeepAlive(CopyFromUri);
					GC::KeepAlive(Checksum);
					GC::KeepAlive(ConflictOld);
					GC::KeepAlive(ConflictNew);
					GC::KeepAlive(ConflictWork);
					GC::KeepAlive(PropertyEditFile);
				}

				if(_lock)
					_lock->Detach(keepProperties);
			}
			finally
			{
				_info = nullptr;
				__super::Detach(keepProperties);
			}
		}
	};
}
