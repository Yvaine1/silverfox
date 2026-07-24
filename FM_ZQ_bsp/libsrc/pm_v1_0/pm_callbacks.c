/******************************************************************************
*
* Copyright (C) 2015-2018 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/
/**
 * @file pm_callbacks.c
 *
 * @addtogroup fpm_apis FmshPM APIs
 * @{
 *****************************************************************************/

#include "pm_callbacks.h"
#include "pm_client.h"

static FPm_Notifier* notifierList = NULL;

/****************************************************************************/
/**
 * @brief  Add notifier into the list
 *
 * @param  notifier Pointer to notifier object which needs to be added
 *  in the list
 *
 * @return Returns FPMST_SUCCESS if notifier is added /
 *  FPMST_INVALID_PARAM if given notifier argument is NULL
 *
 * @note   None
 *
 ****************************************************************************/
FPmStatus FPm_NotifierAdd(FPm_Notifier* const notifier)
{
	FPmStatus status;

	if (NULL == notifier) {
		status = FPMST_INVALID_PARAM;
		goto done;
	}

	notifier->received = 0U;

	/* New notifiers are added at the front of list */
	notifier->next = notifierList;
	notifierList = notifier;

	status = FPMST_SUCCESS;

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Remove notifier from the list
 *
 * @param  notifier Pointer to notifier object to be removed from list
 *
 * @return Returns FPMST_SUCCESS if notifier is removed /
 *  FPMST_INVALID_PARAM if given notifier pointer is NULL /
 *  FPMST_FAILURE if notifier is not found
 *
 * @note   None
 *
 ****************************************************************************/
FPmStatus FPm_NotifierRemove(FPm_Notifier* const notifier)
{
	FPmStatus status = FPMST_FAILURE;
	FPm_Notifier* curr;
	FPm_Notifier* prev = NULL;

	if (NULL == notifier) {
		status = FPMST_INVALID_PARAM;
		goto done;
	}

	curr = notifierList;
	while (curr != NULL) {
		if (notifier == curr) {
			if (prev != NULL) {
				prev->next = curr->next;
			}
			else {
				notifierList = curr->next;
			}

			status = FPMST_SUCCESS;
			break;
		}
		prev = curr;
		curr = curr->next;
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call to process notification event
 *
 * @param  node    Node which is the subject of notification
 * @param  event   Event which is the subject of notification
 * @param  oppoint Operating point of the node in question
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
void FPm_NotifierProcessEvent(const enum FPmNodeId node,
			      const enum FPmNotifyEvent event,
			      const u32 oppoint)
{
	FPm_Notifier* notifier;

	/* Validate the notifier list */
	if (NULL != notifierList) {
		notifier = notifierList;
	}
	else {
		notifier = NULL;
	}
	while (notifier != NULL) {
		if ((node == notifier->node) &&
		    (event == notifier->event)) {
			notifier->oppoint = oppoint;
			notifier->received++;
			if (notifier->callback != NULL) {
				notifier->callback(notifier);
			}
			/*
			 * Don't break here, there could be multiple pairs of
			 * (node, event) with different notifiers
			 */
		}
		notifier = notifier->next;
	}
}
 /** @} */
